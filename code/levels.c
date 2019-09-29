
struct {
    int shapes_spawned;
    b32 played_mid_rotation_sound;
    f32 top_block_p_y;
    int last_lane;
    v2 enemies_p[40];
    f32 rotate_t;
    int rotate_i;
    int post_level_spawn_shapes;
    
    b32 spawn_small_shapes;
    f32 spawn_small_shapes_t;
    f32 spawn_small_shapes_target;
    
    int level_progression; //Should really be an enum @Cleanup
    b32 win;
    b32 rotate_this_frame;
    
} typedef Level_Tetris_State;

struct {
    v2 enemy_dp;
    v2 enemy_p;
    v2 enemy_half_size;
    b32 is_collising_with_wall;
} typedef Level_Pong_State;

struct {
    v2 enemy_p;
    f32 movement_t;
    f32 movement_target;
    b32 is_moving_right;
    b32 move_down;
    b32 do_invader_player_collision_test;
    int num_of_changes;
} typedef Level_Invaders_State;

struct {
    union {
        Level_Pong_State pong;
        Level_Invaders_State invaders;
        Level_Tetris_State tetris;
    };
} typedef Level_State;

Level_State level_state;


struct {
    u32 arena_color;
    u32 wall_color;
} typedef Level_Info;

Level_Info level_info[] = {
    /*L01_NORMAL*/   {0x781D27, 0x44171D},
    /*L02_WALL*/     {0xC25B17, 0x8D3E11},
    /*L05_PONG*/     {0x008080, 0x00ffff},
    /*L03_TETRIS*/   {0x406949, 0x89da59},
    /*L06_INVADERS*/ {0x333333, 0x111111},
};


#define ARENA_COLOR level_info[current_level].arena_color
#define WALL_COLOR level_info[current_level].wall_color

#define BALL_ACTIVE 0x1
#define BALL_DESTROYED_ON_DP_Y_DOWN 0x2

struct {
    u32 flags;
    
    v2 p;
    v2 dp;
    
    v2 desired_p;
    
    f32 half_size;
    f32 base_speed;
    f32 speed_multiplier;
    
    f32 trail_spawner_t;
    int next_trail;
    
    Playing_Sound *sound;
    Playing_Sound *comet_sound;
} typedef Ball;

Ball balls[16];
int next_ball;

struct {
    v2 p;
    v2 dp;
    v2 half_size;
    f32 angle;
    
    f32 life;
    f32 life_d;
    u32 color;
} typedef Particle;

Particle particles[1024];
int next_particle;

int score;
int touchless_bonus;

enum {
    POWER_INACTIVE,
    
    // Powerups
    POWER_INVINCILITY,
    POWER_COMET,
    //POWER_INCREASE_BALL_SIZE
    POWER_TRIPLE_SHOT,
    
    POWERUP_LAST = POWER_TRIPLE_SHOT,
    
    // Powerdowns
    POWER_INSTAKILL,
    POWER_STRONG_BLOCKS,
    POWER_INVERTED_CONTROLS,
    POWER_SLOW_PLAYER,
    
    POWER_COUNT,
} typedef Power_Block_Kind;

inline int
random_powerup() {
    return random_int_in_range(1, POWERUP_LAST);
}

inline int
random_powerdown() {
    return random_int_in_range(POWERUP_LAST+1, POWER_COUNT-1);
}

struct {
    Power_Block_Kind kind;
    v2 p;
    f32 anim_t;
    b32 anim_add_direction;
    v2 half_size;
    Playing_Sound *sound;
} typedef Power_Block;

Power_Block power_blocks[16];
int next_power_block;

#define BLOCK_RIVAL_A 0x1
#define BLOCK_RIVAL_B 0x2

struct {
    union {
        int shape_index;
    };
} typedef Block_Level_Info;

struct Block {
    u32 flags;
    
    v2 relative_p;
    v2 p;
    v2 *parent_p;
    v2 half_size;
    f32 ball_speed_multiplier;
    
    int life;
    u32 color;
    
    Power_Block_Kind power_block;
    
    struct Block *neighbours[4];
    Block_Level_Info level_specific;
    
} typedef Block;

Block blocks[1024];
int num_blocks;
int blocks_destroyed;

v2 arena_half_size;
f32 arena_left_wall_visual_p;
f32 arena_left_wall_visual_dp;
f32 arena_right_wall_visual_p;
f32 arena_right_wall_visual_dp;
f32 arena_top_wall_visual_p;
f32 arena_top_wall_visual_dp;

b32 first_ball_movement = true;
b32 advance_level = false;
int advance_level_target = 0;
b32 lose_life_at_the_end_for_frame = false;

b32 initialized = false;

f32 dt_multiplier = 1.f;

f32 invincibility_t;
f32 invincibility_alpha;
f32 comet_t;
int number_of_triple_shots;

f32 strong_blocks_t;
f32 inverted_controls_t;
f32 slow_player_t;

Level current_level;

inline void
add_screenshake(f32 factor) {
    cam_dp = (v2){random_f32_in_range(-factor, factor), random_f32_in_range(-factor, factor)};
}

inline Particle*
spawn_particle(v2 p, f32 dp_scale, v2 half_size, f32 angle, f32 life, f32 life_d, u32 color) {
    Particle *particle = particles + next_particle++;
    if (next_particle >= array_count(particles)) next_particle = 0;
    
    particle->p = p;
    particle->dp = (v2){random_bilateral()*dp_scale, random_bilateral()*dp_scale};
    particle->half_size = half_size;
    particle->life = life;
    particle->life_d = life_d;
    particle->color = color;
    particle->angle = angle;
    return particle;
}

inline void
spawn_particle_explosion(int count, v2 p, f32 dp_scale, f32 base_size, f32 base_life, u32 color) {
    for (int i = 0; i < count; i++) {
        base_size += random_bilateral() * base_size*.1f;
        base_life += random_bilateral() * base_life*.1f;
        Particle *particle = spawn_particle(p, dp_scale, (v2){base_size, base_size}, random_f32_in_range(0, 360), base_life, 1.f, color);
    }
}

inline void
increase_ball_size(Ball *ball) {
    ball->half_size += .2f/ball->half_size;
}

internal void
reset_and_reverse_ball_dp_y(Ball *ball) {
    if (ball->dp.y > 0) ball->dp.y = -ball->base_speed * ball->speed_multiplier;
    else ball->dp.y = ball->base_speed * ball->speed_multiplier;
}

internal void
reset_balls() {
    for (int i = 0; i < array_count(balls); i++) {
        if (balls[i].sound) set_volume(balls[i].sound, 0.f);
        if (balls[i].comet_sound) set_volume(balls[i].comet_sound, 0.f);
        Playing_Sound *temp = balls[i].sound;
        Playing_Sound *temp2 = balls[i].comet_sound;
        balls[i] = (Ball){0};
        balls[i].sound = temp;
        balls[i].comet_sound = temp2;
    }
}

internal void
do_wall_hit_effects(v2 p, f32 *sound_source) {
    spawn_particle_explosion(10, p, 8.f, 1.f, .3f, 0xffffff);
    play_sound_with_variation(get_sound(S_SPRING), .3f, sound_source);
    play_sound_with_variation(get_sound(S_REDIRECT), .3f, sound_source);
    Playing_Sound *fireworks = play_sound_with_variation(get_sound(S_FIREWORKS), .2f, sound_source);
    set_volume(fireworks, fireworks->target_volume - .6f);
    fireworks->speed_multiplier += .7f;
}

internal void
process_ball_when_dp_y_down(Ball *ball) {
    if (ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN) {
        if (ball->sound) set_volume(ball->sound, 0.f);
        if (ball->comet_sound) set_volume(ball->comet_sound, 0.f);
        play_sound_with_variation(get_sound(S_FIREWORKS), .5f, &ball->p.x);
        ball->flags &= ~BALL_ACTIVE;
    }
    
}

internal Block*
get_next_available_block() {
    Block *result = blocks+num_blocks++;
    *result = (Block){0};
    if (num_blocks >= array_count(blocks)) {
        num_blocks = 0;
    }
    return result;
}

internal Ball*
get_next_available_ball_and_zero() {
    for (Ball *ball = balls; ball != balls + array_count(balls); ball++) {
        if (!(ball->flags & BALL_ACTIVE)) {
            Playing_Sound *temp = ball->sound;
            Playing_Sound *temp2 = ball->comet_sound;
            zero_struct(*ball);
            ball->sound = temp;
            ball->comet_sound = temp2;
            return ball;
        }
    }
    
    invalid_code_path;
    return 0;
}

internal void
spawn_triple_shot_balls() {
    
    Ball *ball;
    for (int i = 0; i < 2; i++) {
        ball = get_next_available_ball_and_zero();
        ball->base_speed = 75;
        ball->p.x = player_visual_p.x;
        ball->p.y = player_visual_p.y + player_half_size.y;
        ball->dp.y = ball->base_speed;
        ball->half_size = .75;
        ball->speed_multiplier = balls[0].speed_multiplier;
        ball->flags = BALL_ACTIVE | BALL_DESTROYED_ON_DP_Y_DOWN;
        ball->dp.x = 45.f;
        play_sound_with_variation(get_sound(S_FIREWORKS), .5f, 0);
    }
    
    ball->dp.x = -45.f;
}

internal void
reset_power_blocks() {
    for (Power_Block *power_block = power_blocks;
         power_block!= power_blocks+array_count(power_blocks);
         power_block++) {
        power_block->kind = POWER_INACTIVE;
        set_volume(power_block->sound, 0.f);
    }
}

internal void
spawn_power_block(Power_Block_Kind kind, v2 p) {
    Power_Block *power_block = power_blocks + next_power_block++;
    if (next_power_block >= array_count(power_blocks)) next_power_block = 0;
    power_block->p = p;
    power_block->kind = kind;
    power_block->anim_add_direction = true;
    power_block->anim_t = 0.f;
    power_block->half_size = (v2){2.f, 2.f};
    power_block->sound->speed_multiplier = 1.f-map_into_range_normalized(-arena_half_size.y, p.y, arena_half_size.y);
}

internal b32
maybe_update_highscore() {
    b32 result = false;
    
    if (score > save_data.levels[current_level].highscore) {
        save_data.levels[current_level].highscore = score;
        result = true;
    }
    
    return result;
}

#define TETRIS_SHAPE_INITIAL_Y 55
#define TETRIS_SHAPE_COUNT 5
#define TETRIS_SHAPE_HEIGHT 4
char *tetris_shapes[][TETRIS_SHAPE_HEIGHT] = {
    "",
    " 00",
    " 0",
    " 0",
    
    "",
    "  0",
    " 00",
    " 0",
    
    "",
    " 0",
    "000",
    "",
    
    "",
    "00",
    "00",
    "",
    
    "0",
    "0",
    "0",
    "0",
    
};

internal void
spawn_tetris_shape(int random_lane, f32 size, b32 neighbours, Block *parent_block) {
    
    Level_Tetris_State *tetris = &level_state.tetris;
    
    int num_lanes = (int)(1.f/size * 20.f);
    
    if (random_lane == -1) {
        random_lane = random_int_in_range(0, num_lanes-1);
        int max_iteration = 10;
        while (random_lane == tetris->last_lane && max_iteration) {
            random_lane = random_int_in_range(0, num_lanes-1);
            max_iteration--;
        }
        
        if (!max_iteration) {
            random_lane++;
            if (random_lane >= num_lanes-1) random_lane = 0;
        }
    }
    
    
    tetris->last_lane = random_lane;
    
    v2 block_half_size = {size, size};
    f32 spacing = .25f;
    
    
    f32 lane_size = (arena_half_size.x*2 -10)/(f32)num_lanes;
    f32 offset = block_half_size.x*2 + spacing;
    f32 x_p = (-arena_half_size.x + 5) + lane_size*.5f + random_lane*lane_size;
    
    char **shape = &tetris_shapes[random_int_in_range(0, TETRIS_SHAPE_COUNT-1)][0];
    
    v2 p = {-offset, offset};
    
    tetris->enemies_p[tetris->shapes_spawned] = (v2){x_p, TETRIS_SHAPE_INITIAL_Y};
    
    
    u8 random_color = (u8)random_int_in_range(200, 255);
    u32 color;
    int choice = random_int_in_range(0, 5);
    if (choice == 0) color= make_color(random_color, 255-random_color, 255-random_color);
    else if (choice == 2) color= make_color(255-random_color, random_color,255-random_color);
    else if (choice == 3) color= make_color(255-random_color, 255-random_color, random_color);else if (choice == 4) color= make_color(255-random_color, (u8)random_int_in_range(200, 255), random_color);
    else color= make_color(255-random_color, random_color, (u8)random_int_in_range(200, 255));
    
    f32 original_x = p.x;
    
    Block *block_neighbours[4] = {0, 0, 0, 0};
    int next_block_neighbours = 0;
    
    for (int i = 0; i < TETRIS_SHAPE_HEIGHT; i++) {
        char *at = shape[i];
        while(*at) {
            if (*at++ != ' ') {
                Block *block = get_next_available_block();
                
                assert(next_block_neighbours < array_count(block_neighbours));
                block_neighbours[next_block_neighbours++] = block;
                
                block->life = 1;
                block->half_size = block_half_size;
                
                block->relative_p = p;
                if (parent_block) {
                    block->p = add_v2(block->relative_p, parent_block->p);
                } else
                    block->p = block->relative_p;
                
                block->color = color;
                block->level_specific.shape_index = tetris->shapes_spawned;
                block->ball_speed_multiplier = min(2.f, 1+ block->level_specific.shape_index*.15f);
                
                if (tetris->shapes_spawned == 1 || tetris->level_progression == 3)
                    block->power_block = random_int_in_range(POWERUP_LAST+1, POWER_COUNT-1);
                else if (tetris->win) {
                    if (random_choice(3)) block->power_block = random_int_in_range(0, POWERUP_LAST);
                }
                
            }
            p.x += offset;
        }
        p.y -= offset;
        if (i != TETRIS_SHAPE_HEIGHT-1) p.x = original_x;
    }
    
    if (neighbours) {
        for (int i = 0; i < next_block_neighbours; i++) {
            int index = 0;
            for (int j = 0; j < next_block_neighbours; j++) {
                if (i == j) continue;
                block_neighbours[i]->neighbours[index] = block_neighbours[j];
                index++;
            }
        }
    }
    
    tetris->shapes_spawned++;
    if (tetris->shapes_spawned == array_count(tetris->enemies_p)-1) { //@Hack
        tetris->spawn_small_shapes = false;
    }
    assert(tetris->shapes_spawned <= array_count(tetris->enemies_p));
    
}

internal b32 // Returns if we should abort the test_for_win_condition proc
level_score_changed() {
    switch(current_level) {
        
        case L06_INVADERS: {
            level_state.invaders.movement_target -= level_state.invaders.movement_target*.0005f;
        } break;
        
        case L03_TETRIS: {
            
            if (blocks_destroyed == num_blocks) {
                
                if (level_state.tetris.level_progression == 3) {
                    level_state.tetris.level_progression++;
                    spawn_tetris_shape(-1, 1.5, true, 0);
                    level_state.tetris.spawn_small_shapes = true;
                    level_state.tetris.spawn_small_shapes_target = .5f;
                    level_state.tetris.win = true;
                }
                
                if (level_state.tetris.shapes_spawned == 1) {
                    level_state.tetris.post_level_spawn_shapes++;
                    return true;
                } else if (level_state.tetris.level_progression == 2) {
                    level_state.tetris.post_level_spawn_shapes += 2;
                    level_state.tetris.rotate_t = 0.f;
                    Playing_Sound *sound = play_sound(get_sound(S_OLD_SOUND), false);
                    set_volume(sound, .2f);
                    level_state.tetris.level_progression++;
                }
            } else if (blocks_destroyed+1 == num_blocks) {
                if (level_state.tetris.shapes_spawned == 2) {
                    level_state.tetris.level_progression++;
                    level_state.tetris.spawn_small_shapes = true;
                    level_state.tetris.spawn_small_shapes_target = 1.f;
                    return true;
                }
            }
            
            return !level_state.tetris.win;
        } break;
    }
    
    return false;
}

internal void
test_for_win_condition() {
    
    blocks_destroyed++;
    score += player_life + touchless_bonus++;
    
    if (level_score_changed()) return;
    
    b32 go_back_to_menu = false;
    
    if (blocks_destroyed == num_blocks) {
        //victory
        advance_level = true;
        advance_level_target = current_level+1;
        b32 should_save_game = false;
        if (current_level < L_COUNT-1) {if (save_data.levels[current_level+1].locked) {
                save_data.levels[current_level+1].locked = false;
                should_save_game = true;
            }
        } else {
            go_back_to_menu = true;
        }
        
        if (maybe_update_highscore() || should_save_game) {
            save_game();
        }
        score = 0;
        
        if (go_back_to_menu) {
            change_game_mode(GM_MENU); // Special screen?
        }
    }
}

internal void
block_destroyed(Block *block, Ball* ball, b32 maybe_destroy_neighbours) {
    block->life = 0;
    
    spawn_particle_explosion(20, block->p, 12.f, min(block->half_size.y, 2.f), .3f, block->color);
    Particle *block_particle = spawn_particle(block->p, 0.f, block->half_size, 0.f, 1.25f, 5.f, block->color);
    block_particle->dp = sub_v2(block->p, ball->p);
    
    test_for_win_condition();
    
    if (block->power_block) {
        spawn_power_block(block->power_block, block->p);
    }
    
    if (maybe_destroy_neighbours) {
        Playing_Sound *sound = play_sound(get_sound(S_BRICK_1)+random_int_in_range(0, 4), false); //@Hardcoded
        set_volume(sound, .2f);
        sound->source_x = &block->p.x;
        Playing_Sound *redirect = play_sound_with_variation(get_sound(S_REDIRECT), .3f, &block->p.x);
        Playing_Sound *fireworks = play_sound_with_variation(get_sound(S_FIREWORKS), .2f, &block->p.x);
        set_volume(fireworks, fireworks->target_volume - .6f);
        fireworks->speed_multiplier += .7f;
        
        if (current_time - last_hit_sound_played < .5f) {
            if (next_hit_sound_to_play < 4)
                next_hit_sound_to_play++; //@Hardcoded
        } else next_hit_sound_to_play = 0;
        play_sound(get_sound(S_HIT_1+next_hit_sound_to_play), false);
        last_hit_sound_played = current_time;
        
        for (int i = 0; i < array_count(block->neighbours); i++) {
            if (!block->neighbours[i]) break;
            if (block->neighbours[i]->life) {
                block->neighbours[i]->life = 0;
                block_destroyed(block->neighbours[i], ball, false);
            }
        }
    }
}

inline b32
do_ball_block_collision(Ball *ball, Block *block, f32 *min_t, Block **hit_block, b32 *ball_update_axis) {
    assert(block->life > 0);
    
    f32 diff = ball->desired_p.y - ball->p.y;
    if (diff != 0) {
        f32 collision_point;
        if (ball->dp.y > 0) collision_point = block->p.y - block->half_size.y - ball->half_size;
        else collision_point = block->p.y + block->half_size.y + ball->half_size;
        f32 t_y = (collision_point - ball->p.y) / diff;
        if (t_y >= 0.f && t_y <= *min_t) {
            
            f32 t_x = lerp(ball->p.x, t_y, ball->desired_p.x);
            if (t_x + ball->half_size > block->p.x - block->half_size.x &&
                t_x - ball->half_size < block->p.x + block->half_size.x) {
                
                *min_t = t_y;
                *hit_block = block;
                *ball_update_axis = 1;
                return true;
            }
        }
    }
    
    diff = ball->desired_p.x - ball->p.x;
    if (diff != 0) {
        f32 collision_point;
        if (ball->dp.x > 0) collision_point = block->p.x - block->half_size.x - ball->half_size;
        else collision_point = block->p.x + block->half_size.x + ball->half_size;
        f32 t_x = (collision_point - ball->p.x) / diff;
        if (t_x >= 0.f && t_x <= *min_t) {
            f32 at_y = lerp(ball->p.y, t_x, ball->desired_p.y);
            if (at_y + ball->half_size > block->p.y - block->half_size.y &&
                at_y - ball->half_size < block->p.y + block->half_size.y) {
                
                *min_t = t_x;
                *hit_block = block;
                *ball_update_axis = 0;
                return true;
            }
        }
    }
    
    return false;
}


internal void
create_invader(v2 p) { //TODO: Center this p
    char *invader[] = {
        "  0     0",
        "   0   0",
        "  0000000",
        " 00 000 00",
        "00000000000",
        "0 0000000 0",
        "0 0     0 0",
        "   00 00 "};
    f32 block_half_size = .8f;
    p.x -= block_half_size*11;
    f32 original_x = p.x;
    
    for (int i = 0; i < array_count(invader); i++) {
        char *at = invader[i];
        while(*at) {
            if (*at++ != ' ') {
                Block *block = get_next_available_block();
                block->life = 1;
                block->half_size = (v2){block_half_size, block_half_size};
                block->relative_p = p;
                block->color = make_color_from_grey(255);
                block->ball_speed_multiplier = 1 + (f32)(array_count(invader)-i)*.5f/array_count(invader);
                
                block->parent_p = &level_state.invaders.enemy_p;
                
                if (random_choice(20)) {
                    block->power_block = random_powerup();
                }
            }
            p.x += block_half_size*2.f;
        }
        p.y -= block_half_size*2.f;
        p.x = original_x;
    }
}

internal void
calculate_all_neighbours() {
    //@Speed: This is stupid
    
    for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
        if (!block->life) return;
        
        for (Block *test_block = blocks; test_block != blocks+array_count(blocks); test_block++) {
            if (!test_block->life) break;
            if (test_block == block) continue;
            
            v2 diff = sub_v2(block->relative_p, test_block->relative_p);
            f32 len = len_sq(diff);
            f32 size = max(block->half_size.x, block->half_size.y);
            if (len <= square(size * 2.2f)) {
                for (int i = 0; i < array_count(block->neighbours); i++) {
                    if (block->neighbours[i]) continue;
                    block->neighbours[i] = test_block;
                    break;
                }
            }
        }
    }
}

internal void
create_block_block(int num_x, int num_y, v2 relative_spacing, f32 x_offset, f32 y_offset, v2 block_half_size, f32 base_speed_multiplier, v2 *parent_p) {
    x_offset += (f32)num_x * block_half_size.x * (2.f+(relative_spacing.x*2.f)) *.5f - block_half_size.x*(1.f+relative_spacing.x);
    y_offset += -2.f;
    for (int y = 0; y < num_y; y++) {
        for (int x = 0; x < num_x; x++) {
            Block *block = get_next_available_block();
            
            block->life = 1;
            block->half_size = block_half_size;
            
            block->relative_p.x = x*block->half_size.x*(2.f+relative_spacing.x*2.f) - x_offset;
            block->relative_p.y = y*block->half_size.y*(2.f+relative_spacing.y*2.f) - y_offset;
            
            block->parent_p = parent_p;
            
            u8 k = (u8)(y*255/num_y);
            block->color = make_color(255, k, 128);
            block->ball_speed_multiplier = base_speed_multiplier + (f32)y*1.25f/(f32)num_y;
        }
        
    }
}

internal void
reset_power() {
    invincibility_t = 0.f;
    comet_t = 0.f;
    number_of_triple_shots = 0;
    
    strong_blocks_t = 0.f;
    inverted_controls_t = 0.f;
    slow_player_t = 0.f;
}

internal void
init_ball(Ball *ball) {
    ball->dp.x = 0.f;
    ball->p.x = 0.f;
    ball->base_speed = 50;
    ball->dp.y = -ball->base_speed;
    ball->p.y = 40;
    ball->half_size = .75;
    ball->speed_multiplier = 1.f;
    ball->flags |= BALL_ACTIVE;
    ball->desired_p = ball->p;
    
}

internal void
start_game(Level level) {
    
    {
        Playing_Sound *sound = play_sound(get_sound(S_START_GAME), false);
        set_volume(sound, .4f);
    }
    
    advance_level = false;
    if (level >= L_COUNT) level = 0;
    else if (level < 0) level = L_COUNT-1;
    
    score = 0;
    player_life = 3;
    current_level = level;
    
    zero_struct(level_state);
    
    reset_balls();
    reset_power_blocks();
    zero_array(blocks);
    zero_array(particles);
    next_particle = 0;
    
    first_ball_movement = true;
    init_ball(balls);
    
    reset_power();
    
    num_blocks = 0;
    blocks_destroyed = 0;
    
    switch(level) {
        case L01_NORMAL: {
            create_block_block(16, 7, (v2){.1f, .1f}, 0.f, 3.f, (v2){4.8f, 2.4f}, 1.f, 0);
        } break;
        
        case L02_WALL: {
            
            int num_x = 21;
            int num_y = 9;
            f32 block_x_half_size = 4.f;
            f32 x_offset = (f32)num_x * block_x_half_size * 2.f *.5f - block_x_half_size*.5f;
            f32 y_offset = 0.f;
            for (int y = 0; y < num_y; y++) {
                for (int x = 0; x < num_x; x++) {
                    Block *block = get_next_available_block();
                    
                    block->life = 1;
                    block->half_size = (v2){block_x_half_size, 2};
                    
                    block->relative_p.x = x*block->half_size.x*2.0f - x_offset;
                    
                    if (y % 2) {
                        if (x == 0) {
                            block->half_size.x *= .5f;
                            block->relative_p.x += block->half_size.x;
                        }
                    } else {
                        if (x == num_x-1)
                            block->half_size.x *= .5f;
                        block->relative_p.x += block->half_size.x;
                    }
                    
                    block->relative_p.y = y*block->half_size.y*2.0f - y_offset;
                    u8 k = (u8)(y*255/num_y);
                    block->color = make_color(k/2, k, 255);
                    block->ball_speed_multiplier = 1+ (f32)y*1.25f/(f32)num_y;
                    
                    if (y <= 2) {
                        if (random_choice(5))
                            block->power_block = random_powerup();
                    } else if (y >= 6) {
                        if (random_choice(6))
                            block->power_block = random_powerdown();
                    }
                }
                
            }
        } break;
        
        case L05_PONG: {
            
            int num_x = 12;
            int num_y = 3;
            create_block_block(num_x, num_y, (v2){0.05f, 0.05f}, 0.f, -28.f, (v2){1.5f, 1.5f} , 2.f, &level_state.pong.enemy_p);
            level_state.pong.enemy_half_size.x = num_x*(1.5f*1.05f);
            
            for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
                if (!block->life) continue;
                if (random_choice(3))
                    block->power_block = random_powerdown();
            }
        } break;
        
        
        case L03_TETRIS: {
            
            spawn_tetris_shape(6, 1.5f, true, 0);
            
        } break;
        
        
        case L06_INVADERS: {
            
            for (int i = 0; i < 3; i++) {
                f32 y = (f32)i*20;
                create_invader((v2){-50, y});
                create_invader((v2){-25, y});
                create_invader((v2){0, y});
                create_invader((v2){25, y});
                create_invader((v2){50, y});
            }
            level_state.invaders.movement_target = 2.f;
            level_state.invaders.enemy_p.x = -25.f;
            level_state.invaders.is_moving_right = true;
            
            calculate_all_neighbours();
        } break;
        
        invalid_default_case;
    }
    
}

internal void
lose_life(b32 instakill) {
    add_screenshake(200);
    
    player_life--;
    if (instakill || !player_life) {
        // game over
        Playing_Sound *sound = play_sound(get_sound(S_GAME_OVER), false);
        set_volume(sound, .35f);
        advance_level = true;
        advance_level_target = current_level;
        return;
    }
    
    lose_life_at_the_end_for_frame = true;
}

internal void
post_simulate_level(Level level, f32 dt) {
    switch(level) {
        case L03_TETRIS: {
            Level_Tetris_State *tetris = &level_state.tetris;
            
            for (int i = 0; i < level_state.tetris.post_level_spawn_shapes; i++) {
                spawn_tetris_shape(-1, 4, false, 0);
            }
            tetris->post_level_spawn_shapes = 0;
            
            if (tetris->level_progression == 1 &&
                tetris->shapes_spawned > 10) {
                tetris->level_progression++;
                tetris->spawn_small_shapes = false;
                tetris->spawn_small_shapes_t = 0.f;
            }
            
        } break;
    }
}

internal void
simulate_level(Level level, f32 dt) {
    switch(level) {
        
        case L06_INVADERS: {
            Level_Invaders_State *invaders = &level_state.invaders;
            invaders->do_invader_player_collision_test = false;
            
            invaders->movement_t += dt;
            if (invaders->movement_t >= invaders->movement_target) {
                invaders->movement_t -= invaders->movement_target;
                
                invaders->movement_target -= invaders->movement_target*.0007f;
                
                Playing_Sound *sound = play_sound(get_sound(S_OLD_SOUND), false);
                set_volume(sound, .2f);
                sound->speed_multiplier = .5f+(level_state.invaders.num_of_changes/200.f);
                
                if (invaders->move_down) {
                    invaders->enemy_p.y -= 2.5;
                    invaders->move_down = false;
                    invaders->do_invader_player_collision_test = true;
                } else {
                    if (invaders->is_moving_right) {
                        invaders->enemy_p.x += 2.5;
                        if (invaders->enemy_p.x > 24.5) { invaders->is_moving_right = !invaders->is_moving_right;
                            invaders->move_down = true;
                        }
                    } else {
                        invaders->enemy_p.x += -2.5;
                        if (invaders->enemy_p.x < -23) { invaders->is_moving_right = !invaders->is_moving_right;
                            invaders->move_down = true;
                        }
                    }
                }
                
                level_state.invaders.num_of_changes++;
            }
        } break;
        
        case L05_PONG: {
            
            Level_Pong_State *pong = &level_state.pong; 
            
            v2 ddp = mul_v2(pong->enemy_dp, -5.f);
            if (absf(balls[0].p.x - pong->enemy_p.x) > 6.f) {
                if (balls[0].p.x > pong->enemy_p.x) ddp.x += 500.f;
                else if (balls[0].p.x < pong->enemy_p.x) ddp.x += -500.f;
            }
            
            v2 desired_dp = add_v2(pong->enemy_dp, mul_v2(ddp, dt));
            v2 desired_p = add_v2(add_v2(pong->enemy_p,
                                         mul_v2(desired_dp, dt)),
                                  mul_v2(ddp, square(dt)));
            
            
            if (desired_p.x > arena_right_wall_visual_p - pong->enemy_half_size.x) {
                desired_p.x = arena_right_wall_visual_p - pong->enemy_half_size.x;
                desired_dp = mul_v2(desired_dp, -.5f);
                if (!pong->is_collising_with_wall) {
                    add_screenshake(150);
                    Playing_Sound *sound = play_sound_with_variation(get_sound(S_PLAYER_WALL), .15f, &pong->enemy_p.x);
                    sound->speed_multiplier -= .5f;
                    set_volume(sound, sound->target_volume+1.f);
                }
                pong->is_collising_with_wall = true;
            }  else if (desired_p.x < arena_left_wall_visual_p + pong->enemy_half_size.x) {
                desired_p.x = arena_left_wall_visual_p + pong->enemy_half_size.x;
                desired_dp = mul_v2(desired_dp, -.5f);
                if (!pong->is_collising_with_wall) {
                    add_screenshake(150);
                    Playing_Sound *sound = play_sound_with_variation(get_sound(S_PLAYER_WALL), .15f, &pong->enemy_p.x);
                    sound->speed_multiplier -= .5f;
                    set_volume(sound, sound->target_volume+1.f);
                    
                }
                pong->is_collising_with_wall = true;
            } else {
                pong->is_collising_with_wall = false;
            }
            
            pong->enemy_dp = desired_dp;
            pong->enemy_p = desired_p;
        } break;
        
        case L03_TETRIS: {
            Level_Tetris_State *tetris = &level_state.tetris;
            tetris->top_block_p_y = -10000.f;
            
            for (int i = 0; i < level_state.tetris.shapes_spawned; i++) {
                tetris->enemies_p[i].y -= (TETRIS_SHAPE_INITIAL_Y*2+tetris->enemies_p[i].y) * .02f*dt;
            }
            
            tetris->rotate_t += dt;
            if (tetris->level_progression == 3) {
                if (tetris->rotate_t >= 1.f) {
                    if (!tetris->played_mid_rotation_sound) {
                        tetris->played_mid_rotation_sound = true;
                        Playing_Sound *sound = play_sound(get_sound(S_OLD_SOUND), false);
                        set_volume(sound, .3f);
                    }
                }
            }
            
            tetris->rotate_this_frame = false;
            if (tetris->rotate_t >= 2.f) {
                
                if (tetris->level_progression == 3) {
                    tetris->played_mid_rotation_sound = false;
                    Playing_Sound *sound = play_sound(get_sound(S_OLD_SOUND), false);
                    tetris->rotate_this_frame = true;
                    set_volume(sound, .3f);
                }
                
                tetris->rotate_t -= 2.f;
                for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
                    if (!block->life) continue;
                    if (block->level_specific.shape_index > 10) {
                        if      (tetris->rotate_i == 0) block->relative_p = (v2){ block->relative_p.y, -block->relative_p.x};
                        else if (tetris->rotate_i == 1) block->relative_p = (v2){ block->relative_p.y,  block->relative_p.x};
                        else if (tetris->rotate_i == 2) block->relative_p = (v2){-block->relative_p.y,  block->relative_p.x};
                        else if (tetris->rotate_i == 3) block->relative_p = (v2){ block->relative_p.y,  block->relative_p.x};
                    }
                }
                tetris->rotate_i++;
                if (tetris->rotate_i >= 4) tetris->rotate_i = 0;
            }
            if (tetris->spawn_small_shapes) {
                tetris->spawn_small_shapes_t += dt;
                if (tetris->spawn_small_shapes_t > tetris->spawn_small_shapes_target) {
                    tetris->spawn_small_shapes_t -= tetris->spawn_small_shapes_target;
                    spawn_tetris_shape(-1, 1.5, true, 0);
                }
            }
            
        } break;
    }
}

internal void
simulate_block_for_level(Block *block, Level level, f32 dt) {
    switch(level) {
        
        case L06_INVADERS: {
            if (block->parent_p) //@Copy'n'Paste
                block->p = add_v2(block->relative_p, *block->parent_p);
            else
                block->p = block->relative_p;
            
            if (level_state.invaders.do_invader_player_collision_test) {
                if (block->p.y - block->half_size.y < player_target_p.y + player_half_size.y) {
                    lose_life(true);
                }
            }
        } break;
        
        
        case L03_TETRIS: {
            block->p = add_v2(block->relative_p, level_state.tetris.enemies_p[block->level_specific.shape_index]);
            
            if (block->p.y-block->half_size.y <
                player_target_p.y + player_half_size.y) {
                lose_life(false);
                block_destroyed(block, balls, true);
            }
            
            if (level_state.tetris.rotate_this_frame) {
                spawn_particle_explosion(10, block->p, 17.f, 2.f, .35f, block->color);
            }
            
            if (block->p.y > level_state.tetris.top_block_p_y)
                level_state.tetris.top_block_p_y = block->p.y;
        } break;
        
        default: {
            if (block->parent_p)
                block->p = add_v2(block->relative_p, *block->parent_p);
            else
                block->p = block->relative_p;
        }
    }
}
