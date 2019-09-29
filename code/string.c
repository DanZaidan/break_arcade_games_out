struct {
    char *data;
    s64 size;
} typedef String;

internal b32
strings_match(String a, String b) {
    if (a.size != b.size) return false;
    
    char *at_a = a.data;
    char *at_b = b.data;
    for (s64 i = 0; i < a.size; i++) {
        if (*at_a != *at_b) return false;
        at_a++;
        at_b++;
    }
    
    return true;
}