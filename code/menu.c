int hot_level = L_COUNT;
b32 use_light_logo = true;

f32 music_sixteenth_note_t;
int sixteenth_note_count;
#define MUSIC_TEMPO 130
#define SIXTEENTH_NOTE_TIME (60.f/(MUSIC_TEMPO*4.f))

u32 menu_text_colors[] = {0xff0080, 0xff2480, 0xff4880, 0xff6d80, 0xff9180, 0xffb680, 0xffda80, 0xffb680, 0xff9180, 0xff6d80, 0xff4880, 0xff2480};
u32 menu_dark_colors[] = {0x00257f, 0x00497f, 0x006e7f, 0x00927f, 0x00b77f, 0x00db7f, 0x00ff7f};
int menu_text_starting_color;

char *level_names[] = {"BREAK\\OUT", "POWER\\BREAKOUT", "\\PONG", "\\TETRIS","SPACE\\INVADERS"};

f32 test;

internal void
update_menu(Input *input, f32 dt) {
    clear_screen(0x112a34);
    draw_rect((v2){0, 30}, (v2){120, 15.25f}, 0x2b454f);
    
    draw_rotated_rect((v2){-arena_half_size.x-20, arena_half_size.y}, (v2){60, 40}, 45.f,  0x15a427);
    draw_rotated_rect((v2){ arena_half_size.x+20, arena_half_size.y}, (v2){60, 40}, -45.f, 0x15a427);
    
    {
        f32 first_p = -70.f;
        f32 range = 140.f;
        
        f32 delta_p = range / (L_COUNT-1);
        v2 p = (v2){first_p, -16};
        if (input->mouse_dp.x != 0 && input->mouse_dp.y != 0) {
            int old_hot = hot_level;
            int new_hot_level = clamp(0, (int)((player_target_p.x-first_p + delta_p*.5)/delta_p), L_COUNT-1);
            if (!save_data.levels[new_hot_level].locked) {
                hot_level = new_hot_level;
                if (old_hot != hot_level) {play_sound(get_sound(S_INTERFACE), false);}
            } else hot_level = L_COUNT;
        }
        
        int align = TEXT_ALIGN_CENTER;
        
        for (int i = 0; i < L_COUNT; i++) {
            //draw_rect(p, (v2){10, 10}, 0x555500);
            u32 color = 0xaaaaaa;
            f32 size = .2f;
            if (save_data.levels[i].locked) {
                color = 0x777777;
                draw_text(level_names[i], p, size, &color, 1, 0, align);
            } else {
                color = 0xaaaaaa;
                if (i == hot_level) {
                    draw_text(level_names[i], p, size, menu_text_colors, array_count(menu_text_colors), menu_text_starting_color, align);
                } else {
                    color = 0xffffff;
                    draw_text(level_names[i], p, size, &color, 1, 0, align);
                }
                draw_number(save_data.levels[i].highscore, add_v2((v2){0, -14}, p), 3.f, color, 1, true);
            }
            
            p.x += delta_p;
        }
    }
    
    if (pressed(BUTTON_LMB)) {
        if (hot_level < L_COUNT && (!save_data.levels[hot_level].locked)) {
            current_level = hot_level;
            change_game_mode(GM_GAMEPLAY);
        }
    }
    
    
    if (use_light_logo) {
        u32 color = 0xffffff;
        draw_bitmap(get_bitmap(B_LOGO_LIGHT), (v2){0, 28}, (v2){28.57f, 2.10f}, 1.f);
        draw_text("BREAK", (v2){1, 42}, .64f, menu_text_colors, array_count(menu_text_colors), array_count(menu_text_colors)/2, TEXT_ALIGN_CENTER);
        draw_rect((v2){0, 20.75f}, (v2){11.25f, 3.1f}, 0x00ff00);
        u32 text_color = 0xff0080;
        draw_text("OUT", (v2){.5f, 22.75}, .25f, &text_color, 1, 0, TEXT_ALIGN_CENTER);
    } else {
        draw_bitmap(get_bitmap(B_LOGO_DARK), (v2){0, 28}, (v2){28.57f, 2.10f}, 1.f);
        draw_text("BREAK", (v2){1, 42}, .64f, menu_dark_colors, array_count(menu_dark_colors), 0, TEXT_ALIGN_CENTER);
        draw_rect((v2){0, 20.75f}, (v2){11.25f, 3.1f}, 0x00ff00);
        u32 text_color = 0x00257f;
        draw_text("OUT", (v2){.5f, 22.75}, .25f, &text_color, 1, 0, TEXT_ALIGN_CENTER);
    }
    
    
    {
        u32 color = 0xfffca2;
        draw_text("CREATED BY DAN ZAIDAN", (v2){0, 13}, .2f, &color, 1, 0, TEXT_ALIGN_CENTER);
        color = 0xffda80;
        draw_text("ON A LIVE STREAM\\WATCH THE DEVELOPMENT AT\\YOUTUBE.COM/DANZAIDAN", (v2){0, 8}, .175f, &color, 1, 0, TEXT_ALIGN_CENTER);
    }
    
}


struct {
    b32 played_spring_sound_effect;
    b32 playing;
    b32 played_block_sound[1024];
} typedef Level_Transition;
global_variable Level_Transition level_transition;


internal void
draw_level_transtion_to_gameplay(f32 t, f32 dt) {
    
    //////////
    // Menu Drawing
    
    f32 menu_t = map_into_range_normalized(.4f, t, 1.f);
    
    clear_screen(lerp_color(ARENA_COLOR, menu_t, 0x112a34));
    draw_transparent_rect((v2){0, 30}, (v2){120, 15.25f}, 0x2b454f, menu_t);
    
    f32 rect_1_p = lerp(-arena_half_size.x-70, menu_t, -arena_half_size.x-20);
    f32 rect_2_p = lerp( arena_half_size.x+70, menu_t,  arena_half_size.x+20);
    
    draw_rotated_rect((v2){rect_1_p, arena_half_size.y}, (v2){60, 40}, 45.f,  0x15a427);
    draw_rotated_rect((v2){rect_2_p, arena_half_size.y}, (v2){60, 40}, -45.f, 0x15a427);
    
    
    ///////////
    // Game Drawing
    
    // Draw blocks
    Block *block = blocks;
    for (int i = num_blocks-1; i >= 0; i--) {
        assert(block->life);
        
        simulate_block_for_level(block, current_level, dt);
        
        v2 p = block->p;
        
        f32 block_t = t - (.5f*((f32)i/(f32)num_blocks));
        if (current_level != L03_TETRIS && block_t <= 0.05f) {
            if (!level_transition.played_block_sound[i]) {
                level_transition.played_block_sound[i] = true;
                if (random_choice(max(1, num_blocks/20))) {
                    Playing_Sound *sound = play_sound(get_sound(S_BRICK_1+random_int_in_range(0, 4)), false); //@Hardcoded
                    set_volume(sound, .2f);
                }
            }
        }
        block_t = max(0.f, block_t); 
        
        p = lerp_v2(p, block_t, add_v2(p, (v2){0, 100}));
        draw_rect_subpixel(p, block->half_size, block->color);
        
        block++;
    }
    
    // Wall movements @Copy'n'Paste
    if (t < 0.5f) {
        if (!level_transition.played_spring_sound_effect) {
            level_transition.played_spring_sound_effect = true;
            Playing_Sound *sound = play_sound(get_sound(S_SPRING), false);
            set_volume(sound, 2.f);
        }
        f32 arena_left_wall_visual_ddp = 150.f*(-arena_half_size.x - arena_left_wall_visual_p) + 7.f*(-arena_left_wall_visual_dp);
        arena_left_wall_visual_dp += arena_left_wall_visual_ddp*dt;
        arena_left_wall_visual_p += arena_left_wall_visual_ddp*square(dt)*.5f +
            arena_left_wall_visual_dp*dt;
        
        f32 arena_right_wall_visual_ddp = 150.f*(arena_half_size.x - arena_right_wall_visual_p) + 7.f*(-arena_right_wall_visual_dp);
        arena_right_wall_visual_dp += arena_right_wall_visual_ddp*dt;
        arena_right_wall_visual_p += arena_right_wall_visual_ddp*square(dt)*.5f +
            arena_right_wall_visual_dp*dt;
        
        f32 arena_top_wall_visual_ddp = 150.f*(arena_half_size.y - arena_top_wall_visual_p) + 7.f*(-arena_top_wall_visual_dp);
        arena_top_wall_visual_dp += arena_top_wall_visual_ddp*dt;
        arena_top_wall_visual_p += arena_top_wall_visual_ddp*square(dt)*.5f +
            arena_top_wall_visual_dp*dt;
        
        draw_arena_rects((v2){0, 0}, arena_left_wall_visual_p, arena_right_wall_visual_p, arena_top_wall_visual_p, WALL_COLOR);
    }
    
}


internal void
draw_level_transtion_to_menu(f32 t, f32 dt) {
    
    clear_screen(lerp_color(ARENA_COLOR, t, 0x112a34));
    
    ///////////
    // Game Drawing
    
    // Draw blocks
    Block *block = blocks;
    for (int i = num_blocks-1; i >= 0; i--, block++) {
        if (!block->life) continue;
        
        simulate_block_for_level(block, current_level, dt);
        draw_rect_subpixel(block->p, block->half_size, block->color);
    }
    
    // Wall movements @Copy'n'Paste
    if (t < 0.5f) {
        if (!level_transition.played_spring_sound_effect) {
            level_transition.played_spring_sound_effect = true;
            Playing_Sound *sound = play_sound(get_sound(S_SPRING), false);
            set_volume(sound, 2.f);
        }
        f32 arena_left_wall_visual_ddp = 150.f*(-arena_half_size.x - arena_left_wall_visual_p) + 7.f*(-arena_left_wall_visual_dp);
        arena_left_wall_visual_dp += arena_left_wall_visual_ddp*dt;
        arena_left_wall_visual_p += arena_left_wall_visual_ddp*square(dt)*.5f +
            arena_left_wall_visual_dp*dt;
        
        f32 arena_right_wall_visual_ddp = 150.f*(arena_half_size.x - arena_right_wall_visual_p) + 7.f*(-arena_right_wall_visual_dp);
        arena_right_wall_visual_dp += arena_right_wall_visual_ddp*dt;
        arena_right_wall_visual_p += arena_right_wall_visual_ddp*square(dt)*.5f +
            arena_right_wall_visual_dp*dt;
        
        f32 arena_top_wall_visual_ddp = 150.f*(arena_half_size.y - arena_top_wall_visual_p) + 7.f*(-arena_top_wall_visual_dp);
        arena_top_wall_visual_dp += arena_top_wall_visual_ddp*dt;
        arena_top_wall_visual_p += arena_top_wall_visual_ddp*square(dt)*.5f +
            arena_top_wall_visual_dp*dt;
        
        draw_arena_rects((v2){0, 0}, arena_left_wall_visual_p, arena_right_wall_visual_p, arena_top_wall_visual_p, WALL_COLOR);
    }
    
    
    
    //////////
    // Menu Drawing
    
    draw_transparent_rect((v2){0, 0}, (v2){200, 200}, 0x112a34, t);
    
    draw_transparent_rect((v2){0, 30}, (v2){120, 15.25f}, 0x2b454f, t);
    
    f32 rect_1_p = lerp(-arena_half_size.x-70, t, -arena_half_size.x-20);
    f32 rect_2_p = lerp( arena_half_size.x+70, t,  arena_half_size.x+20);
    
    draw_rotated_rect((v2){rect_1_p, arena_half_size.y}, (v2){60, 40}, 45.f,  0x15a427);
    draw_rotated_rect((v2){rect_2_p, arena_half_size.y}, (v2){60, 40}, -45.f, 0x15a427);
    
}