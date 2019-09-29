
#include <math.h> // just for sinf, cosf, sqrtf


internal int
clamp(int min, int val, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

internal f32
clampf(f32 min, f32 val, f32 max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

inline f32
lerp(f32 a, f32 t, f32 b) {
    return (1-t)*a + t*b;
}

inline f32
square(f32 a) {
    return a*a;
}

inline f32
absf(f32 a) {
    if (a < 0) return -a;
    return a;
}

inline f32
map_into_range_normalized(f32 min, f32 val, f32 max) {
    f32 range = max - min;
    return clampf(0, (val-min)/range, 1);
}

inline f32
map_into_range_normalized_unclamped(f32 min, f32 val, f32 max) {
    f32 range = max - min;
    return (val-min)/range;
}

inline f32
move_towards(f32 val, f32 target, f32 speed) {
    if (val > target) return clampf(target, val - speed, val);
    if (val < target) return clampf(val, val + speed, target);
    return val;
    
    
}

inline f32
safe_divide_1(f32 a, f32 b) {
    if (b == 0.f) b = 1.f;
    return a/b;
}

#define PI32 3.1415926f

inline f32
deg_to_rad(f32 angle) {
    return PI32/180.f*angle;
}

inline f32
rad_to_deg(f32 angle) {
    return angle/PI32*180.f;
}

inline int
trunc_f32(f32 a) {
    return (int)a;
}

inline int
ceil_f32(f32 a) {
    return (int)(a+1.f);
}

// Color
inline u32
make_color(u8 r, u8 g, u8 b) {
    return (b << 0) |
        (g << 8) |
        (r << 16);
}

inline u32
make_color_from_grey(u8 grey) {
    return (grey << 0) |
        (grey << 8) |
        (grey << 16);
}

inline u32
lerp_color(u32 a, f32 t, u32 b) {
    f32 a_r = (f32)((a & 0xff0000) >> 16);
    f32 a_g = (f32)((a & 0xff00) >> 8);
    f32 a_b = (f32)(a & 0xff);
    
    f32 b_r = (f32)((b & 0xff0000) >> 16);
    f32 b_g = (f32)((b & 0xff00) >> 8);
    f32 b_b = (f32)(b & 0xff);
    
    return make_color((u8)lerp(a_r, t, b_r),
                      (u8)lerp(a_g, t, b_g),
                      (u8)lerp(a_b, t, b_b));
}

// Vector 2

struct {
    union {
        struct {
            f32 x;
            f32 y;
        };
        
        f32 e[2];
    };
} typedef v2;

inline v2
add_v2(v2 a, v2 b) {
    return (v2){a.x + b.x, a.y + b.y};
}

inline v2
sub_v2(v2 a, v2 b) {
    return (v2){a.x - b.x, a.y - b.y};
}

inline v2
mul_v2(v2 a, f32 s) {
    return (v2){a.x*s, a.y*s};
}

inline f32
len_sq(v2 v) {
    return square(v.x) + square(v.y);
}

inline f32
len_v2(v2 v) {
    return sqrtf(square(v.x) + square(v.y));
}

inline v2
normalize(v2 a) {
    return mul_v2(a, 1.f/len_v2(a));
}

inline f32
dot_v2(v2 a, v2 b) {
    return a.x*b.x + a.y*b.y;
}

inline v2
lerp_v2(v2 a, f32 t, v2 b) {
    return (v2){(1-t)*a.x + t*b.x, (1-t)*a.y + t*b.y};
}

// v2i

struct {
    union {
        struct {
            int x;
            int y;
        };
        
        int e[2];
    };
} typedef v2i;

inline v2i
sub_v2i(v2i a, v2i b) {
    return (v2i){a.x - b.x, a.y - b.y};
}


// Random

u32 random_state = 1234;

inline u32
random_u32() {
    u32 result = random_state;
    result ^= result << 13;
    result ^= result >> 17;
    result ^= result << 5;
    random_state = result;
    return result;
}

inline b32
random_b32() {
    return random_u32()%2;
}

inline b32
random_choice(int chance) {
    return random_u32()%chance == 0;
}

inline int
random_int_in_range(int min, int max) { //inclusive
    int range = max - min + 1;
    int result = random_u32() % range;
    result += min;
    return result;
}

inline f32
random_unilateral() {
    return (f32)random_u32() / (f32)MAX_U32;
}

inline f32
random_bilateral() {
    return random_unilateral() * 2.f - 1.f;
}

inline f32
random_f32_in_range(f32 min, f32 max) {
    return random_unilateral() * (max-min) + min;
}

// Matrix 2x2
struct {
    union {
        f32 e[2][2];
        struct {
            f32 _00, _01, _10, _11;
        };
    };
} typedef m2;

inline v2
mul_m2_v2(m2 m, v2 v) {
    return (v2){v.x*m._00 + v.y*m._01,
        v.x*m._10 + v.y*m._11};
}

// Rect2

struct {
    v2 p[4]; // Counter-clockwise points
} typedef Rect2;

inline Rect2
make_rect_min_max(v2 min, v2 max) {
    Rect2 result;
    result.p[0] = min;
    result.p[1] = (v2){max.x, min.y};
    result.p[2] = max;
    result.p[3] = (v2){min.x, max.y};
    return result;
}

inline Rect2
make_rect_center_half_size(v2 c, v2 h) {
    return make_rect_min_max(sub_v2(c, h), add_v2(c, h));
}





inline f32
find_look_at_rotation(v2 a, v2 b) {
    v2 v = sub_v2(a, b);
    return rad_to_deg(atan2f(v.y, v.x));
}
