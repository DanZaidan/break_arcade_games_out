#if PROFILER

internal void draw_text(char *text, v2 p, f32 size, u32 *colors, int color_count, int first_color, int text_align);
internal void draw_f32(f32 num, v2 p, f32 size, b32 color);
internal f32 draw_number(int number, v2 p, f32 size, u32 color, int number_of_leading_zeros, b32 centered);

enum {
    PROFILING_INPUT,
    PROFILING_GAME,
    PROFILING_FLIP,
    
    PROFILING_AUDIO,
    
    PROFILING_DRAW_RECT,
    PROFILING_DRAW_BITMAP,
    PROFILING_DRAW_ROTATED,
    PROFILING_DRAW_BACKGROUND,
    
    PROFILING_COUNT,
} typedef Profiler_Items;

struct {
    int hit_count;
    f32 timer;
    u64 begin_timer;
} typedef Profiling_Data;

Profiling_Data back_profiling_data[PROFILING_COUNT];
Profiling_Data writing_profiling_data[PROFILING_COUNT];

inline void
begin_profiling(Profiler_Items item) {
    writing_profiling_data[item].begin_timer = os_get_perf_counter();
    writing_profiling_data[item].hit_count++;
}

inline void
end_profiling(Profiler_Items item) {
    writing_profiling_data[item].timer += os_seconds_elapsed(writing_profiling_data[item].begin_timer)*1000.f;
}

inline void
render_profiler(f32 dt) {
    v2 p = (v2){65.f, 40.f};
    u32 color = 0xffaa55;
    
    draw_f32(dt*1000.f, (v2){90.f, 45.f}, 2.5f, 0xff0000);
    
#define draw_profiler_guy(name, index) {\
        draw_text(name, p, .15f, &color, 1, 0, 1);\
        f32 hit = (f32)back_profiling_data[index].hit_count;\
        draw_number(back_profiling_data[index].hit_count, add_v2((v2){6, -1}, p), 2.f, color, 1, false);\
        draw_f32(back_profiling_data[index].timer, add_v2((v2){16, -1}, p), 2.f, color);\
        draw_f32(safe_divide_1(back_profiling_data[index].timer,hit), add_v2((v2){24, -1}, p), 2.f, color);\
        p.y -= 3.f;}
    
    draw_profiler_guy("INPU", PROFILING_INPUT);
    draw_profiler_guy("GAME", PROFILING_GAME);
    draw_profiler_guy("FLIP", PROFILING_FLIP);
    color = 0xffddaa;
    draw_profiler_guy("AUDI", PROFILING_AUDIO);
    
    p.y -= 6.f;
    color = 0x4477aa;
    draw_profiler_guy("RECT", PROFILING_DRAW_RECT);
    draw_profiler_guy("BIPM", PROFILING_DRAW_BITMAP);
    draw_profiler_guy("ROTA", PROFILING_DRAW_ROTATED);
    draw_profiler_guy("BACK", PROFILING_DRAW_BACKGROUND);
    
    p.y -= 3.f;
    color = 0x225577;
    draw_text("GAME NO R", p, .15f, &color, 1, 0, 1);
    f32 timer = back_profiling_data[PROFILING_GAME].timer;
    timer -= back_profiling_data[PROFILING_DRAW_RECT].timer;
    timer -= back_profiling_data[PROFILING_DRAW_BITMAP].timer;
    timer -= back_profiling_data[PROFILING_DRAW_ROTATED].timer;
    timer -= back_profiling_data[PROFILING_DRAW_BACKGROUND].timer;
    draw_f32(timer, add_v2((v2){16, -1}, p), 2.f, color);
}

inline void
begin_profiler() {
    for (int i = 0; i < PROFILING_COUNT; i++) {
        back_profiling_data[i] = writing_profiling_data[i];
    }
    zero_array(writing_profiling_data);
}

#else

#define render_profiler(...)
#define begin_profiler(...)
#define begin_profiling(...)
#define end_profiling(...)

#endif