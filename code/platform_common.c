
/////////
// Input

struct {
    b32 is_down;
    b32 changed;
} typedef Button;

enum {
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_UP,
    BUTTON_DOWN,
    
    BUTTON_LMB,
    BUTTON_ESC,
    
    BUTTON_COUNT,
};

struct {
    v2i mouse_dp;
    
    Button buttons[BUTTON_COUNT];
    
} typedef Input;

#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)
#define is_down(b) (input->buttons[b].is_down)


////////////
// Audio

struct {
    int size; //buffer size in bytes
    int channel_count;
    int samples_per_second;
    int bytes_per_sample;
    int running_sample_index;
    
    s16 *samples;
    int samples_to_write;
} typedef Game_Sound_Buffer;


//////////
// Rendering

struct {
    int width, height;
    u32 *pixels;
} typedef Render_Buffer;


/////////
// Platform services to the game

internal String os_read_entire_file(char *file_path);
internal void os_free_file(String s);
internal String os_read_save_file();
internal String os_read_config_file();
internal b32 os_write_save_file(String data);
inline String os_get_pak_data();
internal void os_toggle_fullscreen();

#define OS_JOB_CALLBACK(name) void name(struct Os_Job_Queue *queue, void *data1, void *data2, void *data3)
typedef OS_JOB_CALLBACK(Os_Job_Callback);
internal void os_add_job_to_queue(struct Os_Job_Queue *queue, Os_Job_Callback *callback, void *data1, void* data2, void *data3);

#if PROFILER
inline u64 os_get_perf_counter();
inline f32 os_seconds_elapsed(u64 last_counter);
#endif