#include "utils.c"
#include "string.c"

#include "cooker_common.c"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#include <windows.h>


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

internal b32
write_file(char *file_name, String data) {
    
    b32 result = false;
    
    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle == INVALID_HANDLE_VALUE) {
        assert(0);
        return result;
    }
    
    DWORD bytes_written;
    result = WriteFile(file_handle, data.data, (DWORD)data.size, &bytes_written, 0) && bytes_written == data.size;
    
    CloseHandle(file_handle);
    return result;
    
}

internal void
os_free_file(String s) {
    VirtualFree(s.data, 0, MEM_RELEASE);
}


#if 0

struct {
    u32 *pixels;
    int width, height;
} typedef Bitmap;

internal Bitmap
load_png(char *path) {
    Bitmap result;
    
    String image = os_read_entire_file(path);
    int n;
    stbi_set_flip_vertically_on_load(1);
    result.pixels = (u32*)stbi_load_from_memory((void*)image.data, (int)image.size, &result.width, &result.height, &n, 4);
    u32 *pixel = result.pixels;
    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            u8 r = (u8)(*pixel & 0x0000ff);
            u8 g = (u8)((*pixel & 0x00ff00) >> 8);
            u8 b = (u8)((*pixel & 0xff0000) >> 16);
            u8 a = (u8)((*pixel & 0xff000000) >> 24);
            *pixel++ =  b | (g << 8) | (r << 16) | (a << 24);
        }
    }
    
    os_free_file(image);
    return result;
}

#endif


u8 *memory;
u8 *file_memory_at;
u8 *base_memory;
int used_memory;

#define VERSION_NUMBER 1


internal void
load_asset(char *file_name, u8 asset_format) {
    String file = os_read_entire_file(file_name);
    
    // Where the asset is in the file
    *(u32*)memory = (u32)(file_memory_at-base_memory);
    memory += sizeof(u32);
    
    // The asset format
    *file_memory_at++ = asset_format;
    used_memory++;
    
    // The asset
    memcpy(file_memory_at, file.data, file.size);
    used_memory += (int)file.size;
    file_memory_at += file.size;
}


void main() {
    
    base_memory = malloc(MiB(100));
    memory = base_memory;
    
    *memory++ = 'G';
    *memory++ = 'A';
    used_memory += 2;
    
    *(s16*)memory = VERSION_NUMBER;
    memory += sizeof(s16);
    used_memory += sizeof(s16);
    
    *(s32*)memory = ASSET_COUNT;
    memory += sizeof(s32);
    used_memory += sizeof(s32);
    
    file_memory_at = memory + (ASSET_COUNT+1)*sizeof(u32); // +1 because we insert the total file_size
    used_memory += (ASSET_COUNT+1)*sizeof(s32);
    
    
    // Bitmaps
    load_asset("..\\data\\powerup_invincibility.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerup_triple.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerup_comet.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerdown_inverted.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerdown_instakill.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerdown_slow.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\powerdown_strong.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\half_logo_light.png", ASSET_FORMAT_PNG);
    load_asset("..\\data\\half_logo_dark.png", ASSET_FORMAT_PNG);
    
    // Sounds
    load_asset("..\\data\\breakout_menu.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\breakout_main.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_1.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_2.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_3.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_4.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_5.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_6.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_7.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_8.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_9.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_10.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_11.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_12.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_13.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_14.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_15.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\hit_16.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\game_over.ogg", ASSET_FORMAT_OGG);
    load_asset("..\\data\\sfx\\force_field.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\fireworks_1.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\spring.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\start game.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\lose_life.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\redirect_sound.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\whistle.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\comet_begin.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\comet_loop.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\old_sound.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\powerup_sound.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\powerdown_sound.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\interface.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\player_wall.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\win_sound.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\brick_1.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\brick_2.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\brick_3.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\brick_4.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\brick_5.wav", ASSET_FORMAT_WAV);
    load_asset("..\\data\\sfx\\sine.wav", ASSET_FORMAT_WAV);
    
    
    *(u32*)memory = used_memory;
    
    write_file("..\\data\\assets.pak", (String){(char*)base_memory, used_memory});
}