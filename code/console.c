#if DEVELOPMENT

enum {
    MESSAGE_INT,
    MESSAGE_F32,
    MESSAGE_V2I,
    MESSAGE_V2,
} typedef Message_Kind;

struct {
    Message_Kind kind;
    
    union {
        int int_val;
        f32 f32_val;
        v2i v2i_val;
        v2 v2_val;
    };
    
    f32 timer;
} typedef Message;

Message messages[32];
int current_message;

internal void
print_int(int number) {
    Message *message = messages + current_message++;
    if (current_message >= array_count(messages)) current_message = 0;
    
    message->kind = MESSAGE_INT;
    message->int_val = number;
    message->timer = 2.f;
}

internal void
print_f32(f32 number) {
    Message *message = messages + current_message++;
    if (current_message >= array_count(messages)) current_message = 0;
    
    message->kind = MESSAGE_F32;
    message->f32_val = number;
    message->timer = 2.f;
}


internal void
print_v2i(v2i number) {
    Message *message = messages + current_message++;
    if (current_message >= array_count(messages)) current_message = 0;
    
    message->kind = MESSAGE_V2I;
    message->v2i_val = number;
    message->timer = 2.f;
}


internal void
print_v2(v2 number) {
    Message *message = messages + current_message++;
    if (current_message >= array_count(messages)) current_message = 0;
    
    message->kind = MESSAGE_V2;
    message->v2_val = number;
    message->timer = 2.f;
}


internal void
draw_messages(f32 dt) {
    f32 original_x = -76;
    v2 p = {original_x, 40};
    
    for (int i = 0 ; i < array_count(messages); i++) {
        Message *message = messages + i;
        if (message->timer <= 0.f) continue;
        
        message-> timer -= dt;
        switch(message->kind) {
            case MESSAGE_INT: {
                draw_number(message->int_val, p, 2.5f, 0xffffff, 1, false);
            } break;
            
            case MESSAGE_F32: {
                draw_f32(message->f32_val, p, 2.5f, 0xffffff);
            } break;
            
            case MESSAGE_V2I: {
                draw_number(message->v2i_val.x, p, 2.5f, 0xffffff, 1, false);
                draw_number(message->v2i_val.y, add_v2(p, (v2){10, 0}), 2.5f, 0xffffff, 1, false);
            } break;
            
            case MESSAGE_V2: {
                draw_f32(message->v2_val.x, p, 2.5f, 0xffffff);
                draw_f32(message->v2_val.y, add_v2(p, (v2){10, 0}), 2.5f, 0xffffff);
            } break;
            
            invalid_default_case;
        }
        
        p.x = original_x;
        p.y -= 3.f;
    }
}

#else

#define draw_messages(...)
#define print_int(...)
#define print_f32(...)

#endif