internal void
eat_whitespaces(String *s) {
    for (s64 i = 0; i < s->size;) {
        if (*s->data != ' ' && *s->data != '\r' && *s->data != '\n' && *s->data != '\t') return;
        s->data++;
        s->size--;
    }
}

internal String
consume_next_word(String *s) {
    eat_whitespaces(s);
    String result = *s;
    result.size = 0;
    for (s64 i = 0; i < s->size;) {
        if (*s->data == ' ' || *s->data == '\r' || *s->data == '\n' || *s->data == '\t') {
            eat_whitespaces(s);
            return result;
        }
        s->data++;
        s->size--;
        result.size++;
    }
    
    eat_whitespaces(s);
    return result;
}

internal f32
parse_float_value(String *s) {
    
    consume_next_word(s);
    
    String float_string = consume_next_word(s);
    f32 result = 0.f;
    
    int decimal = 0;
    for (s64 i = 0; i < float_string.size; i++) {
        if (float_string.data[i] >= '0' && float_string.data[i] <= '9') {
            int digit = float_string.data[i] - '0';
            if (decimal == 0) {
                result *= 10.f;
                result += (f32)digit;
            } else {
                result += (f32)digit / (f32)decimal;
                decimal *= 10;
            }
            
        } else if (float_string.data[i] == '.' || float_string.data[i] == ',') {
            decimal = 10;;
        } else return 1.f;
    }
    
    return result;
}

internal b32
parse_b32_value(String *s) {
    consume_next_word(s);
    String val = consume_next_word(s);
    
    String true_string = {"true", 4};
    if (strings_match(val, true_string)) {
        return true;
    }
    
    return false;
}


internal void
load_config() {
    String file = os_read_config_file();
    
    String mouse_sensitivity_string = {"mouse_sensitivity", 17};
    String windowed_string = {"windowed", 8};
    String lock_fps_string = {"lock_fps", 8};
    
    while (file.size) {
        String keyword = consume_next_word(&file);
        if (strings_match(keyword, mouse_sensitivity_string)) {
            f32 value = parse_float_value(&file);
            mouse_sensitivity = clampf(.1f, value, 10.f);
        } else if (strings_match(keyword, windowed_string)) {
            b32 value = parse_b32_value(&file);
            if (value) os_toggle_fullscreen();
        } else if (strings_match(keyword, lock_fps_string)) {
            lock_fps = parse_b32_value(&file);
        }
    }
}
