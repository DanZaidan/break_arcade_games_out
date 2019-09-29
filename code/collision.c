
// Overlapping
inline b32
aabb_vs_aabb(v2 p1, v2 half_size1, v2 p2, v2 half_size2) {
    return p1.y + half_size1.y > p2.y - half_size2.y &&
        p1.y - half_size1.y < p2.y + half_size2.y &&
        p1.x + half_size1.x > p2.x - half_size2.x &&
        p1.x - half_size1.x < p2.x + half_size2.x;
}
