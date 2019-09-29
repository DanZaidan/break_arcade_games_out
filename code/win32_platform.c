#include "utils.c"
#include "math.c"
#include "string.c"

#include "platform_common.c"

#include <windows.h>

// Global Variables for the game
global_variable Render_Buffer render_buffer;
global_variable f32 current_time;
global_variable b32 lock_fps = true;
global_variable struct Os_Job_Queue *general_queue;

// Compiler specific intrinsic
#define interlocked_compare_exchange(a, b, c) InterlockedCompareExchange((volatile long*)a, b, c)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#define STBI_ASSERT assert
#include "stb_image.h"

#include "wav_importer.h"
#include "ogg_importer.h"

#include "cooker_common.c"
#include "asset_loader.c"

#include "profiler.c"
#include "software_rendering.c"
#include "console.c"
#include "audio.c"
#include "game.c"

HWND win32_window;
b32 win32_lock_mouse = true;
Input win32_input = {0};
v2i win32_center_screen;
BITMAPINFO win32_bitmap_info;
f32 frequency_counter;
b32 win32_paused = false;
HCURSOR win32_cursor;
v2i win32_old_mouse_p;
WINDOWPLACEMENT win32_window_position;
#define WINDOW_FLAGS WS_VISIBLE|WS_OVERLAPPEDWINDOW

#define HIDE_CURSOR 1

/////////
// File IO

internal void
os_free_file(String s) {
    VirtualFree(s.data, 0, MEM_RELEASE);
}

internal String
os_read_entire_file(char *file_path) {
    String result = {0};
    
    HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle == INVALID_HANDLE_VALUE) {
        CloseHandle(file_handle);
        return result;
    }
    
    DWORD file_size = GetFileSize(file_handle, 0);
    result.size = file_size;
    result.data = VirtualAlloc(0, result.size,
                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    
    DWORD bytes_read;
    if (ReadFile(file_handle, result.data, file_size, &bytes_read, 0) && file_size == bytes_read) {
        // Success;
        
    } else {
        // @Incomplete: error message?
        assert(0);
    }
    
    CloseHandle(file_handle);
    return result;
}

internal String
os_read_save_file() {
    return os_read_entire_file("data\\save.brk");
}

internal String
os_read_config_file() {
    return os_read_entire_file("config.txt");
}

internal b32
os_write_save_file(String data) {
    b32 result = false;
    
    HANDLE file_handle = CreateFileA("data\\save.brk", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle == INVALID_HANDLE_VALUE) {
        assert(0);
        return result;
    }
    
    DWORD bytes_written;
    result = WriteFile(file_handle, data.data, (DWORD)data.size, &bytes_written, 0) && bytes_written == data.size;
    
    CloseHandle(file_handle);
    return result;
}

inline String
os_get_pak_data() {
    return os_read_entire_file("data\\assets.pak");
}

//////////
// Multi-threading

struct {
    void *data1;
    void *data2;
    void *data3;
    Os_Job_Callback *callback;
} typedef Os_Job;

struct Os_Job_Queue {
    // Platform independent part
    u32 volatile next_entry_to_read;
    u32 volatile next_entry_to_write;
    // completed_jobs
    Os_Job entries[32];
    
    // Platform dependent part
    HANDLE semaphore;
} typedef Os_Job_Queue;

internal b32 //should sleed
win32_do_next_queue_job(Os_Job_Queue *queue) {
    u32 volatile original_next = queue->next_entry_to_read;
    u32 volatile new_entry_to_read = (original_next+1) % array_count(queue->entries);
    
    
    if (original_next != queue->next_entry_to_write) {
        
        u32 interlocked_value = interlocked_compare_exchange(&queue->next_entry_to_read, new_entry_to_read, original_next);
        if (interlocked_value == original_next) {
            Os_Job *entry = queue->entries + interlocked_value;
            entry->callback(queue, entry->data1, entry->data2, entry->data3);
            return false;
        }
    }
    
    return true;
}

DWORD WINAPI win32_thread_proc(void *data) {
    Os_Job_Queue *queue = (Os_Job_Queue*)data;
    
    for (;;) {
        if (win32_do_next_queue_job(queue)) {
            WaitForSingleObjectEx(queue->semaphore, INFINITE, FALSE);
        }
    }
}

internal void
win32_create_queue(Os_Job_Queue *queue, int number_of_thread) {
    
    zero_struct(*queue);
    queue->semaphore = CreateSemaphoreExA(0, 0, number_of_thread, 0, 0, SEMAPHORE_ALL_ACCESS);
    
    for (int i = 0; i < number_of_thread; i++) {
        HANDLE thread = CreateThread(0, 0, win32_thread_proc, queue, 0, 0);
        CloseHandle(thread);
    }
}



internal void
os_add_job_to_queue(Os_Job_Queue *queue, Os_Job_Callback *callback, void *data1, void* data2, void *data3) {
    u32 volatile new_next_entry_to_write = (queue->next_entry_to_write + 1)%array_count(queue->entries);
    assert(new_next_entry_to_write != queue->next_entry_to_read); // Fails with a small queue
    
    Os_Job *entry = queue->entries + queue->next_entry_to_write;
    entry->callback = callback;
    entry->data1 = data1;
    entry->data2 = data2;
    entry->data3 = data3;
    _WriteBarrier();
    queue->next_entry_to_write = new_next_entry_to_write;
    ReleaseSemaphore(queue->semaphore, 1, 0);
}


///////////
// Time
inline f32
os_seconds_elapsed(u64 last_counter) {
    LARGE_INTEGER current_counter;
    QueryPerformanceCounter(&current_counter);
    return (f32)(current_counter.QuadPart - last_counter) / frequency_counter;
}

inline u64
os_get_perf_counter() {
    LARGE_INTEGER current_counter;
    QueryPerformanceCounter(&current_counter);
    return current_counter.QuadPart;
}


//////////
// Audio

#include <dsound.h>

typedef HRESULT Direct_Sound_Create (LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);

global_variable LPDIRECTSOUNDBUFFER win32_sound_buffer;
global_variable Game_Sound_Buffer sound_buffer;

internal void
win32_init_audio(HWND window) {
    HMODULE dsound_dll = LoadLibraryA("dsound.dll");
    if (!dsound_dll) {assert(0); return;};
    
    Direct_Sound_Create *direct_sound_create = (Direct_Sound_Create*)GetProcAddress(dsound_dll, "DirectSoundCreate");
    if (!direct_sound_create) {assert(0); return;};
    
    LPDIRECTSOUND direct_sound = 0;
    if (SUCCEEDED(direct_sound_create(0, &direct_sound, 0))) {
        
        if (SUCCEEDED(direct_sound->lpVtbl->SetCooperativeLevel(direct_sound, window, DSSCL_PRIORITY))) {
            
            // Sound spec
            sound_buffer.channel_count = 2;
            sound_buffer.samples_per_second = 44100;
            sound_buffer.bytes_per_sample = sound_buffer.channel_count*sizeof(s16);
            sound_buffer.size = sound_buffer.samples_per_second*sound_buffer.bytes_per_sample;
            
            
            DSBUFFERDESC buffer_description = {0};
            buffer_description.dwSize = sizeof(buffer_description);
            buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
            LPDIRECTSOUNDBUFFER primary_buffer;
            direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &primary_buffer, 0);
            
            WAVEFORMATEX wave_format = {0};
            wave_format.wFormatTag = WAVE_FORMAT_PCM;
            wave_format.nChannels = (WORD)sound_buffer.channel_count;
            wave_format.nSamplesPerSec = sound_buffer.samples_per_second;
            wave_format.wBitsPerSample = 16;
            wave_format.nBlockAlign = wave_format.nChannels*wave_format.wBitsPerSample/8;
            wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec*wave_format.nBlockAlign;
            
            
            buffer_description = (DSBUFFERDESC){0};
            buffer_description.dwSize = sizeof(buffer_description);
            buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS;
            buffer_description.dwBufferBytes = sound_buffer.size;
            buffer_description.lpwfxFormat = &wave_format;
            
            
            
            if (SUCCEEDED(direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &win32_sound_buffer, 0))) {
                // Success
            } else {
                invalid_code_path;
            }
        } else {invalid_code_path};
    } else {invalid_code_path};
}

internal void
win32_clear_sound_buffer() {
    
    void *region_1;
    DWORD region_1_size;
    void *region_2;
    DWORD region_2_size;
    
    if (SUCCEEDED(win32_sound_buffer->lpVtbl->Lock(win32_sound_buffer, 0, sound_buffer.size, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
        
        s16 *at = region_1;
        DWORD region_1_sample_count = region_1_size/sound_buffer.bytes_per_sample;
        for (DWORD i = 0; i < region_1_sample_count; i++) {
            
            *at++ = 0;
            *at++ = 0;
        }
        
        DWORD region_2_sample_count = region_2_size/sound_buffer.bytes_per_sample;
        at = region_2;
        for (DWORD i = 0; i < region_2_sample_count; i++) {
            
            *at++ = 0;
            *at++ = 0;
        }
        
        if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Unlock(win32_sound_buffer, region_1, region_1_size, region_2, region_2_size))) {
            invalid_code_path;
        }
        
    } else {invalid_code_path;}
    
}


internal void
win32_fill_sound_buffer(DWORD byte_to_lock, DWORD bytes_to_write) {
    
    void *region_1;
    DWORD region_1_size;
    void *region_2;
    DWORD region_2_size;
    
    if (SUCCEEDED(win32_sound_buffer->lpVtbl->Lock(win32_sound_buffer, byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
        
        s16 *dest = region_1;
        s16 *source = sound_buffer.samples;
        DWORD region_1_sample_count = region_1_size/sound_buffer.bytes_per_sample;
        for (DWORD i = 0; i < region_1_sample_count; i++) {
            
            *dest++ = *source++;
            *dest++ = *source++;
            
            sound_buffer.running_sample_index++;
        }
        
        DWORD region_2_sample_count = region_2_size/sound_buffer.bytes_per_sample;
        dest = region_2;
        for (DWORD i = 0; i < region_2_sample_count; i++) {
            
            *dest++ = *source++;
            *dest++ = *source++;
            
            sound_buffer.running_sample_index++;
        }
        
        if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Unlock(win32_sound_buffer, region_1, region_1_size, region_2, region_2_size))) {
            invalid_code_path;
        }
        
    } else {invalid_code_path};
}

OS_JOB_CALLBACK(win32_update_audio) {
    f32 target_dt = .0166f;
    
    LARGE_INTEGER last_counter;
    QueryPerformanceCounter(&last_counter);
    
    for (;;) {
        begin_profiling(PROFILING_AUDIO);
        
        DWORD safety_bytes = (int)((f32)(sound_buffer.samples_per_second*sound_buffer.bytes_per_sample)*target_dt*1);
        safety_bytes -= safety_bytes % sound_buffer.bytes_per_sample;
        
        DWORD play_cursor, write_cursor;
        win32_sound_buffer->lpVtbl->GetCurrentPosition(win32_sound_buffer, &play_cursor, &write_cursor);
        
        DWORD byte_to_lock = (sound_buffer.running_sample_index*sound_buffer.bytes_per_sample) % sound_buffer.size;
        
        DWORD expected_bytes_per_tick = (DWORD)((f32)(sound_buffer.samples_per_second*sound_buffer.bytes_per_sample)*target_dt);
        expected_bytes_per_tick -= expected_bytes_per_tick % sound_buffer.bytes_per_sample;
        DWORD expected_boundary_byte = play_cursor + expected_bytes_per_tick;
        
        DWORD safe_write_buffer = write_cursor;
        if (safe_write_buffer < play_cursor) {
            safe_write_buffer += sound_buffer.size;
        } else {
            safe_write_buffer += safety_bytes;
        }
        
        DWORD target_cursor;
        if (safe_write_buffer < expected_boundary_byte) {
            target_cursor = expected_boundary_byte + expected_bytes_per_tick;
        } else {
            target_cursor = write_cursor + expected_bytes_per_tick + safety_bytes;
        }
        target_cursor %= sound_buffer.size;
        
        DWORD bytes_to_write;
        if (byte_to_lock > target_cursor) {
            bytes_to_write = sound_buffer.size - byte_to_lock + target_cursor;
        } else {
            bytes_to_write = target_cursor - byte_to_lock;
        }
        
        if (bytes_to_write) {
            sound_buffer.samples_to_write = bytes_to_write/sound_buffer.bytes_per_sample;
            assert(bytes_to_write % 4 == 0);
            memset(sound_buffer.samples, 0, sound_buffer.samples_to_write*sizeof(s16)*sound_buffer.channel_count);
            update_game_audio(&sound_buffer, target_dt);
            win32_fill_sound_buffer(byte_to_lock, bytes_to_write);
        }
        
        end_profiling(PROFILING_AUDIO);
        
        f32 elapsed_time = os_seconds_elapsed(last_counter.QuadPart);
        QueryPerformanceCounter(&last_counter);
        int sleep = max(0, (int)(1000.f*(target_dt-elapsed_time))-1);
        Sleep(sleep);
    }
}

/////////
// Misc


internal void
os_toggle_fullscreen() {
    
    //This follow Raymund Chen's "How do I switch a window between normal and fullscreen
    //https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353/
    DWORD style = GetWindowLong(win32_window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(win32_window, &win32_window_position) &&
            GetMonitorInfo(MonitorFromWindow(win32_window,
                                             MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(win32_window, GWL_STYLE,
                          style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(win32_window, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(win32_window, GWL_STYLE, WINDOW_FLAGS);
        SetWindowPlacement(win32_window, &win32_window_position);
        SetWindowPos(win32_window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


/////////
// Callback and Main


internal LRESULT
window_callback (HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    
    LRESULT result = 0;
    
    switch(message) {
        case WM_CLOSE:
        case WM_DESTROY: {
            running = false;
        } break;
        
        case WM_SIZE: {
            RECT rect;
            GetClientRect(window, &rect);
            render_buffer.width = rect.right - rect.left;
            render_buffer.height = rect.bottom - rect.top;
            
            win32_center_screen.x = (rect.right - rect.left)/2;
            win32_center_screen.y = (rect.bottom - rect.top)/2;
            if (win32_lock_mouse) SetCursorPos(win32_center_screen.x, win32_center_screen.y);
            
            win32_old_mouse_p = win32_center_screen;
            
            // (HIDE_CURSOR) SetCursor(0);
            
            if (render_buffer.pixels) {
                VirtualFree(render_buffer.pixels, 0, MEM_RELEASE);
            }
            
            render_buffer.pixels = VirtualAlloc(0, sizeof(u32)*render_buffer.width*render_buffer.height,
                                                MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            
            //fill the bitmapinfo
            win32_bitmap_info.bmiHeader.biSize = sizeof(win32_bitmap_info.bmiHeader);
            win32_bitmap_info.bmiHeader.biWidth = render_buffer.width;
            win32_bitmap_info.bmiHeader.biHeight = render_buffer.height;
            win32_bitmap_info.bmiHeader.biPlanes = 1;
            win32_bitmap_info.bmiHeader.biBitCount = 32;
            win32_bitmap_info.bmiHeader.biCompression = BI_RGB;
        } break;
        
        case WM_ACTIVATEAPP: {
            if (w_param) {
                win32_lock_mouse = true;
                if (HIDE_CURSOR) SetCursor(0);
            } else {
                win32_lock_mouse = false;
                SetCursor(win32_cursor); // Test more throughly
            }
            
            win32_input = (Input){0};
        } break;
        
        default: {
            result = DefWindowProcA(window, message, w_param, l_param);
        }
    }
    
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            LPSTR lpCmdLine, int nShowCmd) {
    
    win32_cursor = LoadCursorA(hInstance, IDC_ARROW);
    timeBeginPeriod(1);
    
    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = window_callback;
    window_class.lpszClassName = "Game_Window_Class";
    
    RegisterClassA(&window_class);
    
    win32_window = CreateWindowExA(0, window_class.lpszClassName, "Break Arcade Games Out", WINDOW_FLAGS, 0, 0, 1280, 720, 0, 0, 0, 0);
    HDC hdc = GetDC(win32_window);
    
#if !DEVELOPMENT
    os_toggle_fullscreen();
#endif
    
    int refresh_rate = GetDeviceCaps(hdc, VREFRESH);
    
    f32 last_dt = 1.f / (f32)refresh_rate;
    f32 target_dt = last_dt;
    
    LARGE_INTEGER frequency_counter_large;
    QueryPerformanceFrequency(&frequency_counter_large);
    frequency_counter = (f32)frequency_counter_large.QuadPart;
    
    assert(win32_center_screen.x != 0);
    win32_old_mouse_p = win32_center_screen;
    
    /////////////
    // Audio stuff
    {
        win32_init_audio(win32_window);
        win32_clear_sound_buffer();
        if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Play(win32_sound_buffer, 0, 0, DSBPLAY_LOOPING))) {
            invalid_code_path;
        }
        sound_buffer.samples = VirtualAlloc(0, sound_buffer.size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        Os_Job_Queue audio_queue;
        win32_create_queue(&audio_queue, 1);
        os_add_job_to_queue(&audio_queue, win32_update_audio, 0, 0, 0);
    }
    
    Os_Job_Queue general_queue_;
    general_queue = &general_queue_;
    win32_create_queue(general_queue, 2);
    
    LARGE_INTEGER last_counter;
    QueryPerformanceCounter(&last_counter);
    
    random_state = (int)last_counter.QuadPart;
    
    while (running) {
        //Input
        
        begin_profiler();
        begin_profiling(PROFILING_INPUT);
        
        for (int i = 0; i < BUTTON_COUNT; i++) win32_input.buttons[i].changed = false;
        
        MSG message;
        while (PeekMessageA(&message, win32_window, 0, 0, PM_REMOVE)) {
            
            switch(message.message) {
                
                case WM_LBUTTONDOWN: {
                    win32_input.buttons[BUTTON_LMB].is_down = true;
                    win32_input.buttons[BUTTON_LMB].changed = true;
                } break;
                
                case WM_LBUTTONUP: {
                    win32_input.buttons[BUTTON_LMB].is_down = false;
                    win32_input.buttons[BUTTON_LMB].changed = true;
                } break;
                
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    
                    u32 vk_code = (u32)message.wParam;
                    b32 was_down = ((message.lParam & (1 << 30)) != 0);
                    b32 is_down  = ((message.lParam & (1 << 31)) == 0);
                    b32 alt_down = message.lParam & (1 << 29);
                    
#define process_button(vk, b) \
                    if (vk_code == vk) {\
                        win32_input.buttons[b].changed = is_down != win32_input.buttons[b].is_down;\
                        win32_input.buttons[b].is_down = is_down;\
                    }
                    
                    process_button(VK_LEFT, BUTTON_LEFT);
                    process_button(VK_RIGHT, BUTTON_RIGHT);
                    process_button(VK_UP, BUTTON_UP);
                    process_button(VK_DOWN, BUTTON_DOWN);
                    process_button(VK_ESCAPE, BUTTON_ESC);
                    if (vk_code == VK_F4 && alt_down) {
                        //save??
                        running = false;
                        break;
                    }
#if DEVELOPMENT
                    if (vk_code == VK_F1 &&
                        is_down && is_down != was_down) {
                        win32_paused = !win32_paused;
                        win32_lock_mouse = !win32_paused;
                        if (HIDE_CURSOR) SetCursor(win32_cursor);
                        ShowCursor(TRUE);
                        break;
                    }
#endif
                    if (vk_code == VK_F11 &&
                        is_down && is_down != was_down) {
                        os_toggle_fullscreen();
                        break;
                    }
                    
                } break;
                
                default: {
                    TranslateMessage(&message); 
                    DispatchMessage(&message);
                }
            }
            
        }
        
        // Mouse Input
        {
            POINT mouse_pointer;
            GetCursorPos(&mouse_pointer);
            mouse_pointer.y = render_buffer.height-mouse_pointer.y;
            
            v2i new_mouse_p = (v2i){mouse_pointer.x, mouse_pointer.y};
            win32_input.mouse_dp = sub_v2i(new_mouse_p, win32_old_mouse_p);
            
            if (win32_lock_mouse) {SetCursorPos(win32_center_screen.x, win32_center_screen.y);
                win32_old_mouse_p = win32_center_screen;
            } else {
                win32_old_mouse_p = new_mouse_p;
            }
        }
        
        if (HIDE_CURSOR) SetCursor(win32_cursor);
        
        end_profiling(PROFILING_INPUT);
        begin_profiling(PROFILING_GAME);
        
        //Simulation
        if (!win32_paused) update_game(&win32_input, last_dt);
        
        end_profiling(PROFILING_GAME);
        
        if (!win32_paused) render_profiler(last_dt);
        
        begin_profiling(PROFILING_FLIP);
        
        //Render
        StretchDIBits(hdc, 0, 0, render_buffer.width, render_buffer.height, 0, 0, render_buffer.width, render_buffer.height, render_buffer.pixels, &win32_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
        
        
        //Get the frame time
        f32 dt_before_sleep = min(.1f, os_seconds_elapsed(last_counter.QuadPart));
        
        end_profiling(PROFILING_FLIP);
        
        if (lock_fps) {
            int sleep = (int)((target_dt-dt_before_sleep)*1000.f);
            if (sleep > 1) {
                Sleep(sleep-1);
            }
        }
        
        last_dt= min(.1f, os_seconds_elapsed(last_counter.QuadPart));
        current_time += last_dt;
        QueryPerformanceCounter(&last_counter);
    }
    
}