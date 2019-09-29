
global_variable v2 cam_p, cam_dp;
global_variable f32 cam_scale = 0.01f;

internal void
clear_screen(u32 color) {
    begin_profiling(PROFILING_DRAW_BACKGROUND);
    
    u32 *pixel = render_buffer.pixels;
    
    for (int y = 0; y < render_buffer.height; y++) {
        for (int x = 0; x < render_buffer.width; x++) {
            *pixel++ = color;
        }
    }
    
    end_profiling(PROFILING_DRAW_BACKGROUND);
}

internal void
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
    
    x0 = clamp(0, x0, render_buffer.width);
    x1 = clamp(0, x1, render_buffer.width);
    y0 = clamp(0, y0, render_buffer.height);
    y1 = clamp(0, y1, render_buffer.height);
    
    u32 *row = render_buffer.pixels + x0 + render_buffer.width*y0;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            *pixel++ = color;
        }
        row += stride;
        pixel = row;
    }
    
}

internal void
draw_rect_in_pixels_transparent(int x0, int y0, int x1, int y1, u32 color, f32 alpha) {
    
    alpha = clampf(0.f, alpha, 1.f);
    
    x0 = clamp(0, x0, render_buffer.width);
    x1 = clamp(0, x1, render_buffer.width);
    y0 = clamp(0, y0, render_buffer.height);
    y1 = clamp(0, y1, render_buffer.height);
    
    u32 *row = render_buffer.pixels + x0 + render_buffer.width*y0;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            *pixel++ = lerp_color(*pixel, alpha, color); //@Speed
        }
        row += stride;
        pixel = row;
    }
    
}

inline f32
calculate_aspect_multipler() {
    //@Clenaup only needs to be done when size changes
    f32 aspect_multiplier = (f32)render_buffer.height;
    if ((f32)render_buffer.width / (f32)render_buffer.height < 1.77f)
        aspect_multiplier = (f32)render_buffer.width / 1.77f;
    
    return aspect_multiplier;
}


internal v2
pixels_dp_to_world(v2i pixels_coord) {
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    
    v2 result;
    result.x = (f32)pixels_coord.x;
    result.y = (f32)pixels_coord.y;
    
    result.x /= aspect_multiplier;
    result.x /= cam_scale;
    
    result.y /= aspect_multiplier;
    result.y /= cam_scale;
    
    return result;
}


internal v2
pixels_to_world(v2i pixels_coord) {
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    
    v2 result;
    result.x = (f32)pixels_coord.x - (f32)render_buffer.width*.5f;
    result.y = (f32)pixels_coord.y - (f32)render_buffer.height*.5f;
    
    result.x /= aspect_multiplier;
    result.x /= cam_scale;
    
    result.y /= aspect_multiplier;
    result.y /= cam_scale;
    
    return result;
}

internal void
draw_rect(v2 p, v2 half_size, u32 color) {
    
    begin_profiling(PROFILING_DRAW_RECT);
    
    p = sub_v2(p, cam_p);
    f32 aspect_multiplier = calculate_aspect_multipler();
    
    half_size.x *= aspect_multiplier * cam_scale;
    half_size.y *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    int x0 = (int)(p.x-half_size.x);
    int y0 = (int)(p.y-half_size.y);
    int x1 = (int)(p.x+half_size.x);
    int y1 = (int)(p.y+half_size.y);
    
    draw_rect_in_pixels(x0, y0, x1, y1, color);
    
    end_profiling(PROFILING_DRAW_RECT);
}

internal void
draw_transparent_rect(v2 p, v2 half_size, u32 color, f32 alpha) {
    
    alpha = clampf(0, alpha, 1);
    
    begin_profiling(PROFILING_DRAW_RECT);
    
    p = sub_v2(p, cam_p);
    f32 aspect_multiplier = calculate_aspect_multipler();
    
    half_size.x *= aspect_multiplier * cam_scale;
    half_size.y *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    int x0 = (int)(p.x-half_size.x);
    int y0 = (int)(p.y-half_size.y);
    int x1 = (int)(p.x+half_size.x);
    int y1 = (int)(p.y+half_size.y);
    
    draw_rect_in_pixels_transparent(x0, y0, x1, y1, color, alpha);
    
    end_profiling(PROFILING_DRAW_RECT);
}

internal void
draw_arena_rects(v2 p, f32 left_most, f32 right_most, f32 half_size_y, u32 color) {
    begin_profiling(PROFILING_DRAW_BACKGROUND);
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    p = sub_v2(p, cam_p);
    
    half_size_y *= aspect_multiplier * cam_scale;
    left_most   *= aspect_multiplier * cam_scale;
    right_most  *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    int x0 = (int)(p.x+left_most);
    int y0 = (int)(p.y-half_size_y);
    int x1 = (int)(p.x+right_most);
    int y1 = (int)(p.y+half_size_y);
    
    draw_rect_in_pixels(0, 0, x0, render_buffer.height, color);
    draw_rect_in_pixels(x1, 0, render_buffer.width, render_buffer.height, color);
    
    draw_rect_in_pixels(x0, y1, x1, render_buffer.height, color);
    
    end_profiling(PROFILING_DRAW_BACKGROUND);
}

internal void
clear_arena_screen(v2 p, f32 left_most, f32 right_most, f32 half_size_y, u32 color) {
    begin_profiling(PROFILING_DRAW_BACKGROUND);
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    p = sub_v2(p, cam_p);
    
    half_size_y *= aspect_multiplier * cam_scale;
    left_most *= aspect_multiplier * cam_scale;
    right_most *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    int x0 = (int)(p.x+left_most);
    int y0 = (int)(p.y-half_size_y);
    int x1 = (int)(p.x+right_most);
    int y1 = (int)(p.y+half_size_y);
    
    draw_rect_in_pixels(x0, 0, x1, y1, color);
    
    end_profiling(PROFILING_DRAW_BACKGROUND);
}

internal f32
draw_number(int number, v2 p, f32 size, u32 color, int number_of_leading_zeros, b32 centered_text) {
    
    f32 square_size = size / 5.f;
    f32 half_square_size = size / 10.f;
    
    b32 draw_minus = false;
    if (number < 0) {
        number *= -1;
        draw_minus = true;
    }
    
    if (centered_text) {
        
        f32 last_move = 0.f;
        int digit = number % 10;
        int n = number;
        int zeros = number_of_leading_zeros;
        while (n || zeros > 0) {
            zeros--;
            if (digit == 1) last_move = square_size;
            else last_move = square_size*2.f;
            
            p.x += last_move;
            
            n /= 10;
            digit = n % 10;
        }
        p.x -= last_move+square_size;
    }
    
    int digit = number % 10;
    
    while (number || number_of_leading_zeros > 0) {
        number_of_leading_zeros--;
        
        switch(digit) {
            case 0: {
                draw_rect((v2){p.x-square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x+square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x, p.y+square_size*2.f}, (v2){half_square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y-square_size*2.f}, (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 1: {
                draw_rect((v2){p.x+square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
                p.x -= square_size*2.f;
            } break;
            
            case 2: {
                draw_rect((v2){p.x, p.y+square_size*2.f}, (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y},                 (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y-square_size*2.f}, (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x+square_size, p.y+square_size}, (v2){half_square_size, half_square_size}, color);
                draw_rect((v2){p.x-square_size, p.y-square_size}, (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 3: {
                draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x-half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x-half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x+square_size, p.y}, (v2){half_square_size, 2.5f*square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 4: {
                draw_rect((v2){p.x+square_size, p.y},             (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x-square_size, p.y+square_size}, (v2){half_square_size, 1.5f*square_size}, color);
                draw_rect((v2){p.x, p.y},                         (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 5: {
                draw_rect((v2){p.x, p.y+square_size*2.f}, (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y},                 (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y-square_size*2.f}, (v2){1.5f*square_size, half_square_size}, color);
                draw_rect((v2){p.x-square_size, p.y+square_size}, (v2){half_square_size, half_square_size}, color);
                draw_rect((v2){p.x+square_size, p.y-square_size}, (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 6: {
                draw_rect((v2){p.x+half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x+half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x+half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x-square_size, p.y}, (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x+square_size, p.y-square_size},          (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 7: {
                draw_rect((v2){p.x+square_size, p.y},             (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 8: {
                draw_rect((v2){p.x-square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x+square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x, p.y+square_size*2.f}, (v2){half_square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y-square_size*2.f}, (v2){half_square_size, half_square_size}, color);
                draw_rect((v2){p.x, p.y},                         (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            case 9: {
                draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x-half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x-half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
                draw_rect((v2){p.x+square_size, p.y},                      (v2){half_square_size, 2.5f*square_size}, color);
                draw_rect((v2){p.x-square_size, p.y+square_size},          (v2){half_square_size, half_square_size}, color);
                p.x -= square_size*4.f;
            } break;
            
            invalid_default_case;
        }
        
        number /= 10;
        digit = number % 10;
    }
    
    if (draw_minus) draw_rect((v2){p.x, p.y}, (v2){2.f*half_square_size, half_square_size}, color);
    
    return p.x;
}


internal void
draw_f32(f32 num, v2 p, f32 size, b32 color) {
    int whole = (int)num;
    int frac = (int)(100.f*(absf(num)-whole));
    
    p.x = draw_number(frac, p, size, color, 2, false);
    draw_rect(add_v2(p, (v2){.5f, -1.f}), (v2){.25f, .25f}, color);
    p.x -= 1.f;
    draw_number(whole, p, size, color, 1, false);
}


internal void
draw_transparent_rotated_rect(v2 p, v2 half_size, f32 angle, u32 color, f32 alpha) { //In degrees
    begin_profiling(PROFILING_DRAW_ROTATED);
    
    alpha = clampf(0, alpha, 1);
    p = sub_v2(p, cam_p);
    angle = deg_to_rad(angle);
    
    f32 cos = cosf(angle);
    f32 sin = sinf(angle);
    
    v2 x_axis = (v2){cos, sin};
    v2 y_axis = (v2){-sin, cos};
    
    m2 rotation = (m2){ //@Speed @Clenaup: Maybe do a single matrix multiplication?
        x_axis.x, y_axis.x,
        x_axis.y, y_axis.y,
    };
    
    Rect2 rect = make_rect_center_half_size((v2){0, 0}, half_size);
    
    for (int i = 0; i < 4; i++) 
        rect.p[i] = mul_m2_v2(rotation, rect.p[i]);
    
    
    // Change to pixels
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    f32 s = aspect_multiplier * cam_scale;
    
    m2 world_to_pixels_scale_transform = (m2){
        s, 0,
        0, s,
    };
    
    v2 position_v = (v2){(f32)render_buffer.width * .5f,
        (f32)render_buffer.height * .5f};
    
    v2i min_bound = (v2i){render_buffer.width, render_buffer.height};
    v2i max_bound = (v2i){0, 0};
    
    
    for (int i = 0; i < 4; i++) {
        rect.p[i] = add_v2(p, rect.p[i]);
        rect.p[i] = mul_m2_v2(world_to_pixels_scale_transform, rect.p[i]);
        rect.p[i] = add_v2(rect.p[i], position_v);
        
        int x_t = trunc_f32(rect.p[i].x);
        int y_t = trunc_f32(rect.p[i].y);
        int x_c = ceil_f32(rect.p[i].x);
        int y_c = ceil_f32(rect.p[i].y);
        
        if (x_t < min_bound.x) min_bound.x = x_t;
        if (x_c > max_bound.x) max_bound.x = x_c;
        if (y_t < min_bound.y) min_bound.y = y_t;
        if (y_c > max_bound.y) max_bound.y = y_c;
    }
    
    min_bound.x = clamp(0, min_bound.x, render_buffer.width);
    max_bound.x = clamp(0, max_bound.x, render_buffer.width);
    min_bound.y = clamp(0, min_bound.y, render_buffer.height);
    max_bound.y = clamp(0, max_bound.y, render_buffer.height);
    
    
    // In pixels
    
    v2 axis_1 = sub_v2(rect.p[1], rect.p[0]);
    v2 axis_2 = sub_v2(rect.p[3], rect.p[0]);
    f32 axis_1_len = len_v2(axis_1);
    f32 axis_2_len = len_v2(axis_2);
    axis_1 = normalize(axis_1);
    axis_2 = normalize(axis_2);
    
    f32 b_r = (f32)((color & 0xff0000) >> 16);
    f32 b_g = (f32)((color & 0xff00) >> 8);
    f32 b_b = (f32)(color & 0xff);
    
    f32 source_r = alpha*b_r;
    f32 source_g = alpha*b_g;
    f32 source_b = alpha*b_b;
    f32 inv_alpha = 1-alpha;
    
    u32 *row = render_buffer.pixels + min_bound.x+1 + render_buffer.width*min_bound.y+1;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = min_bound.y+1; y < max_bound.y; y++) {
        for (int x = min_bound.x+1; x < max_bound.x; x++) {
            
            f32 pixel_p_rel_x = (f32)x - rect.p[0].x;
            f32 pixel_p_rel_y = (f32)y - rect.p[0].y;
            
            f32 proj_1 = pixel_p_rel_x*axis_1.x + pixel_p_rel_y*axis_1.y;
            f32 proj_2 = pixel_p_rel_x*axis_2.x + pixel_p_rel_y*axis_2.y;
            
            if (proj_1 >= 0 &&
                proj_1 <= axis_1_len &&
                proj_2 >= 0 &&
                proj_2 <=  axis_2_len) {
                
                f32 a_r = (f32)((*pixel & 0xff0000) >> 16);
                f32 a_g = (f32)((*pixel & 0xff00) >> 8);
                f32 a_b = (f32)(*pixel & 0xff);
                
                u8 r = (u8)(inv_alpha*a_r + source_r);
                u8 g = (u8)(inv_alpha*a_g + source_g);
                u8 b = (u8)(inv_alpha*a_b + source_b);
                
                u32 pixel_color = b | (g << 8) | (r << 16);
                *pixel = pixel_color;
                
            }
            
            pixel++;
        }
        row += stride;
        pixel = row;
    }
    
    
#if 0
    for (int y = min_bound.y; y < max_bound.y; y++) {
        u32 *pixel = render_buffer.pixels + min_bound.x + render_buffer.width*y;
        
        int x1 = (int)(((f32)y - b)/a); // a = axis.y/axis.x; b is dor x = 0, where is y
        
        for (int x = x1; x < x2; x++) {
            *pixel = lerp_color(*pixel, alpha, color); //@Speed
            pixel++;
        }
        
        /*
        int min_x = ?
            int max_x = ?*/
    }
#endif
    
    end_profiling(PROFILING_DRAW_ROTATED);
}


internal void
draw_rotated_rect(v2 p, v2 half_size, f32 angle, u32 color) { //In degrees
    begin_profiling(PROFILING_DRAW_ROTATED);
    
    angle = deg_to_rad(angle);
    p = sub_v2(p, cam_p);
    
    f32 cos = cosf(angle);
    f32 sin = sinf(angle);
    
    v2 x_axis = (v2){cos, sin};
    v2 y_axis = (v2){-sin, cos};
    
    m2 rotation = (m2){ //@Speed @Clenaup: Maybe do a single matrix multiplication?
        x_axis.x, y_axis.x,
        x_axis.y, y_axis.y,
    };
    
    Rect2 rect = make_rect_center_half_size((v2){0, 0}, half_size);
    
    for (int i = 0; i < 4; i++) 
        rect.p[i] = mul_m2_v2(rotation, rect.p[i]);
    
    
    // Change to pixels
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    f32 s = aspect_multiplier * cam_scale;
    
    m2 world_to_pixels_scale_transform = (m2){
        s, 0,
        0, s,
    };
    
    v2 position_v = (v2){(f32)render_buffer.width * .5f,
        (f32)render_buffer.height * .5f};
    
    v2i min_bound = (v2i){render_buffer.width, render_buffer.height};
    v2i max_bound = (v2i){0, 0};
    
    
    for (int i = 0; i < 4; i++) {
        rect.p[i] = add_v2(p, rect.p[i]);
        rect.p[i] = mul_m2_v2(world_to_pixels_scale_transform, rect.p[i]);
        rect.p[i] = add_v2(rect.p[i], position_v);
        
        int x_t = trunc_f32(rect.p[i].x);
        int y_t = trunc_f32(rect.p[i].y);
        int x_c = ceil_f32(rect.p[i].x);
        int y_c = ceil_f32(rect.p[i].y);
        
        if (x_t < min_bound.x) min_bound.x = x_t;
        if (x_c > max_bound.x) max_bound.x = x_c;
        if (y_t < min_bound.y) min_bound.y = y_t;
        if (y_c > max_bound.y) max_bound.y = y_c;
    }
    
    min_bound.x = clamp(0, min_bound.x, render_buffer.width);
    max_bound.x = clamp(0, max_bound.x, render_buffer.width);
    min_bound.y = clamp(0, min_bound.y, render_buffer.height);
    max_bound.y = clamp(0, max_bound.y, render_buffer.height);
    
    
    // In pixels
    
    v2 axis_1 = sub_v2(rect.p[1], rect.p[0]);
    v2 axis_2 = sub_v2(rect.p[3], rect.p[0]);
    f32 axis_1_len = len_v2(axis_1);
    f32 axis_2_len = len_v2(axis_2);
    axis_1 = normalize(axis_1);
    axis_2 = normalize(axis_2);
    
    u32 *row = render_buffer.pixels + min_bound.x+1 + render_buffer.width*min_bound.y+1;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = min_bound.y+1; y < max_bound.y; y++) {
        for (int x = min_bound.x+1; x < max_bound.x; x++) {
            
            f32 pixel_p_rel_x = (f32)x - rect.p[0].x;
            f32 pixel_p_rel_y = (f32)y - rect.p[0].y;
            
            f32 proj_1 = pixel_p_rel_x*axis_1.x + pixel_p_rel_y*axis_1.y;
            f32 proj_2 = pixel_p_rel_x*axis_2.x + pixel_p_rel_y*axis_2.y;
            
            if (proj_1 >= 0 &&
                proj_1 <= axis_1_len &&
                proj_2 >= 0 &&
                proj_2 <=  axis_2_len) {
                *pixel = color;
            }
            
            pixel++;
        }
        row += stride;
        pixel = row;
    }
    
    
#if 0
    for (int y = min_bound.y; y < max_bound.y; y++) {
        u32 *pixel = render_buffer.pixels + min_bound.x + render_buffer.width*y;
        
        int x1 = (int)(((f32)y - b)/a); // a = axis.y/axis.x; b is dor x = 0, where is y
        
        for (int x = x1; x < x2; x++) {
            *pixel = lerp_color(*pixel, alpha, color); //@Speed
            pixel++;
        }
        
        /*
        int min_x = ?
            int max_x = ?*/
    }
#endif
    
    end_profiling(PROFILING_DRAW_ROTATED);
}



internal void
draw_bitmap(Bitmap *bitmap, v2 p, v2 half_size, f32 alpha_multiplier) {
    
    begin_profiling(PROFILING_DRAW_BITMAP);
    p = sub_v2(p, cam_p);
    alpha_multiplier = clampf(0.f, alpha_multiplier, 1.f)/255.f;
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    
    half_size.x *= aspect_multiplier * cam_scale;
    half_size.y *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    int x0 = (int)(p.x-half_size.x);
    int y0 = (int)(p.y-half_size.y);
    int x1 = (int)(p.x+half_size.x);
    int y1 = (int)(p.y+half_size.y);
    
    
    // In Pixels
    
    x0 = clamp(0, x0, render_buffer.width);
    x1 = clamp(0, x1, render_buffer.width);
    y0 = clamp(0, y0, render_buffer.height);
    y1 = clamp(0, y1, render_buffer.height);
    
    f32 range_x = (f32)(x1 - x0);
    f32 range_y = (f32)(y1 - y0);
    
    // our UV fetch is wrong when the bitmap goes partially outside the area (it gets stretched not considering the 
    
    u32 *row = render_buffer.pixels + x0 + render_buffer.width*y0;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = y0; y < y1; y++) {
        
        f32 v = ((f32)y-y0)/range_y;
        int uv_stride = (int)(v*(f32)bitmap->height)*bitmap->width;
        u32 *source_pixels = bitmap->pixels + uv_stride;
        for (int x = x0; x < x1; x++) {
            
            f32 u = ((f32)x-x0)/range_x;
            
            int pixel_x = (int)(u*(f32)bitmap->width);
            
            u32 bitmap_color = *(source_pixels + pixel_x);
            
            f32 alpha = (f32)((bitmap_color & 0xff000000) >> 24)*alpha_multiplier;
            assert(alpha >= 0.f && alpha <= 255.f);
            
            // color lerp with alpha
            f32 a_r = (f32)((*pixel & 0xff0000) >> 16);
            f32 a_g = (f32)((*pixel & 0xff00) >> 8);
            f32 a_b = (f32)(*pixel & 0xff);
            
            f32 b_r = (f32)((bitmap_color & 0xff0000) >> 16);
            f32 b_g = (f32)((bitmap_color & 0xff00) >> 8);
            f32 b_b = (f32)(bitmap_color & 0xff);
            
            u8 r = (u8)((1-alpha)*a_r + alpha*b_r);
            u8 g = (u8)((1-alpha)*a_g + alpha*b_g);
            u8 b = (u8)((1-alpha)*a_b + alpha*b_b);
            
            u32 pixel_color = b | (g << 8) | (r << 16);
            *pixel++ = pixel_color;
        }
        row += stride;
        pixel = row;
    }
    
    end_profiling(PROFILING_DRAW_BITMAP);
}

internal void print_int(int a);

internal void
draw_rect_subpixel(v2 p, v2 half_size, u32 color) {
    
    f32 aspect_multiplier = calculate_aspect_multipler();
    p = sub_v2(p, cam_p);
    
    half_size.x *= aspect_multiplier * cam_scale;
    half_size.y *= aspect_multiplier * cam_scale;
    
    p.x *= aspect_multiplier * cam_scale;
    p.y *= aspect_multiplier * cam_scale;
    
    p.x += (f32)render_buffer.width * .5f;
    p.y += (f32)render_buffer.height * .5f;
    
    f32 x0f = p.x-half_size.x +.5f;
    int x0 = (int)(x0f);
    f32 x0_alpha = x0f - (f32)x0;
    
    f32 y0f = p.y-half_size.y +.5f;
    int y0 = (int)(y0f);
    f32 y0_alpha = y0f - (f32)y0;
    
    f32 x1f = p.x+half_size.x +.5f;
    int x1 = (int)(x1f);
    f32 x1_alpha = x1f - (f32)x1;
    
    f32 y1f = p.y+half_size.y +.5f;
    int y1 = (int)(y1f);
    f32 y1_alpha = y1f - (f32)y1;
    
    draw_rect_in_pixels_transparent(x0, y0+1, x0+1, y1, color, 1.f-x0_alpha);
    draw_rect_in_pixels_transparent(x1, y0+1, x1+1, y1, color, x1_alpha);
    
    draw_rect_in_pixels_transparent(x0+1, y0, x1, y0+1, color, 1.f-y0_alpha);
    draw_rect_in_pixels_transparent(x0+1, y1, x1, y1+1, color, y1_alpha);
    draw_rect_in_pixels(x0+1, y0+1, x1, y1, color);
}



////////////
// Text

#define TEXT_HEIGHT 7
int letter_spacings[] = {4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 5, 5, 4, 4, 5, 4, 4, 3, 4, 5, 5, 5, 5, 4, 1, 4};
char *letter_table[][TEXT_HEIGHT] = {
    " 00",
    "0  0",
    "0  0",
    "0000",
    "0  0",
    "0  0",
    "0  0",
    
    "000",
    "0  0",
    "0  0",
    "000",
    "0  0",
    "0  0",
    "000",
    
    " 000",
    "0",
    "0",
    "0",
    "0",
    "0",
    " 000",
    
    "000",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "000",
    
    "0000",
    "0",
    "0",
    "000",
    "0",
    "0",
    "0000",
    
    "0000",
    "0",
    "0",
    "000",
    "0",
    "0",
    "0",
    
    " 000",
    "0",
    "0",
    "0 00",
    "0  0",
    "0  0",
    " 000",
    
    "0  0",
    "0  0",
    "0  0",
    "0000",
    "0  0",
    "0  0",
    "0  0",
    
    "000",
    " 0",
    " 0",
    " 0",
    " 0",
    " 0",
    "000",
    
    " 000",
    "   0",
    "   0",
    "   0",
    "0  0",
    "0  0",
    " 000",
    
    "0  0",
    "0  0",
    "0 0",
    "00",
    "0 0",
    "0  0",
    "0  0",
    
    "0",
    "0",
    "0",
    "0",
    "0",
    "0",
    "0000",
    
    "00 00",
    "0 0 0",
    "0 0 0",
    "0   0",
    "0   0",
    "0   0",
    "0   0",
    
    "00  0",
    "0 0 0",
    "0 0 0",
    "0 0 0",
    "0 0 0",
    "0 0 0",
    "0  00",
    
    "0000",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "0000",
    
    " 000",
    "0  0",
    "0  0",
    "000",
    "0",
    "0",
    "0",
    
    " 000 ",
    "0   0",
    "0   0",
    "0   0",
    "0 0 0",
    "0  0 ",
    " 00 0",
    
    "000",
    "0  0",
    "0  0",
    "000",
    "0  0",
    "0  0",
    "0  0",
    
    " 000",
    "0",
    "0 ",
    " 00",
    "   0",
    "   0",
    "000 ",
    
    "000",
    " 0",
    " 0",
    " 0",
    " 0",
    " 0",
    " 0",
    
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    "0  0",
    " 00",
    
    "0   0",
    "0   0",
    "0   0",
    "0   0",
    "0   0",
    " 0 0",
    "  0",
    
    "0   0 ",
    "0   0",
    "0   0",
    "0 0 0",
    "0 0 0",
    "0 0 0",
    " 0 0 ",
    
    "0   0",
    "0   0",
    " 0 0",
    "  0",
    " 0 0",
    "0   0",
    "0   0",
    
    "0   0",
    "0   0",
    " 0 0",
    "  0",
    "  0",
    "  0",
    "  0",
    
    "0000",
    "   0",
    "  0",
    " 0",
    "0",
    "0",
    "0000",
    
    "",
    "",
    "",
    "",
    "",
    "",
    "0",
    
    "   0",
    "  0",
    "  0",
    " 0",
    " 0",
    "0",
    "0",
    
    
};

inline int
get_letter_index(char c) {
    if (c == '.') return 'Z'-'A'+1;
    if (c == '/') return 'Z'-'A'+2;
    return c-'A';
}

enum {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_CENTER,
};

// This is not 100%
internal f32
get_word_align_offset(int text_align, char *word, f32 size) {
    assert(*word);
    if (text_align == TEXT_ALIGN_LEFT) return 0.f;
    
    f32 result = 0.f;
    f32 block_offset_x = size*1.6f*2.f + size*.8f; //@Copy'n'Paste
    
    for (char *at = word; *at && *at != '\\'; at++) {
        if (*at == ' ') {
            result += block_offset_x*4 + size*2.f;
            continue;
        }
        
        int letter_index = get_letter_index(*at);
        result += block_offset_x*letter_spacings[letter_index] + size*2.f;
    }
    
    result -=  size*2.f;
    
    if (text_align == TEXT_ALIGN_RIGHT) return -result;
    else if (text_align == TEXT_ALIGN_CENTER) return -result*.5f;
    
    invalid_code_path;
    return 0.f;
}

internal void
draw_text(char *text, v2 p, f32 size, u32 *colors, int color_count, int first_color, int text_align) {
    
    int next_color = first_color;
    u32 color = colors[next_color];
    
    f32 first_x = p.x;
    p.x += get_word_align_offset(text_align, text, size);
    
    f32 original_x = p.x;
    f32 original_y = p.y;
    v2 half_size = {size*1.6f, size};
    f32 block_offset_x = size*1.6f*2.f + size*.8f; //@Copy'n'Paste
    
    
    for (char *letter_at = text; *letter_at; letter_at++) {
        if (*letter_at == ' ') {
            p.x += block_offset_x*4 + size*2.f;
            original_x = p.x;
            continue;
        } else if (*letter_at == '\\') {
            p.x = first_x;
            p.x += get_word_align_offset(text_align, letter_at+1, size);
            original_x = p.x;
            p.y -= size*(TEXT_HEIGHT+2)*2.5f;
            original_y = p.y;
            continue;
        }
        
        int letter_index = get_letter_index(*letter_at);
        char **letter = &letter_table[letter_index][0];
        
        
        for (int i = 0; i < TEXT_HEIGHT; i++) {
            char *at = letter[i];
            while(*at) {
                if (*at++ != ' ') {
                    draw_rect(p, half_size, color);
                }
                p.x += block_offset_x;
            }
            p.y -= size*2.f + size*.4f;
            if (i != TEXT_HEIGHT-1) p.x = original_x;
            
            if (++next_color >= color_count) next_color = 0;
            color = colors[next_color];
        }
        
        p.x = original_x;
        p.y = original_y;
        p.x += block_offset_x*letter_spacings[letter_index] + size*2.f;
        original_x = p.x;
        
        next_color = first_color;
        color = colors[next_color];
    }
}