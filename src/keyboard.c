#include "header/keyboard.h"
#include "header/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

static struct KeyboardDriverState keyboard_state = {
    .keyboard_input_on = true,
    .keyboard_buffer = {0},
    .read_idx = 0,
    .write_idx = 0,
    .shift_left = false,
    .shift_right = false,
    .caps_lock = false
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

 const char shifted_char[101] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '"',
    0, 0, 0, 0, '<', '_', '>', '?', ')', '!', 
    '@', '#', '$', '%', '^', '&', '*', '(', 0, ':',
    0, '+', 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, '{', '|', '}', 0, 0, '~'
 };

void keyboard_state_activate(void){
    keyboard_state.keyboard_input_on = true;
    memset(keyboard_state.keyboard_buffer, '\0', sizeof(keyboard_state.keyboard_buffer));
}

void keyboard_state_deactivate(void){
    keyboard_state.keyboard_input_on = true;
}

void get_keyboard_buffer(char *buf){
    if(keyboard_state.write_idx != keyboard_state.read_idx){
        memcpy(buf, &keyboard_state.keyboard_buffer[keyboard_state.read_idx], 1);
        keyboard_state.read_idx = (keyboard_state.read_idx + 1) % 256;
    }else{
        *buf = 0;
    }
    // memset(keyboard_state.keyboard_buffer, 0, 256);
}

bool is_shift(){
    return (keyboard_state.shift_left || keyboard_state.shift_right);
}

void keyboard_isr(void) {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    // TODO : Implement scancode processing
    if(keyboard_state.keyboard_input_on){
        char mapped_char = keyboard_scancode_1_to_ascii_map[scancode];
        switch (scancode){
        case 0x3A:
            keyboard_state.caps_lock = !keyboard_state.caps_lock;
            break;
        case 0x36:
            keyboard_state.shift_right = true;
            break;
        case 0xB6:
            keyboard_state.shift_right = false;
            break;
        case 0x2A:
            keyboard_state.shift_left = true;
            break;
        case 0xAA:
            keyboard_state.shift_left = false;
            break;
        default:
            break;
        }

        if((keyboard_state.caps_lock ^ is_shift()) && (mapped_char >= 'a' && mapped_char <= 'z')){
            mapped_char -= 32;
        }
        if(is_shift() && shifted_char[(uint8_t)mapped_char] != 0 && mapped_char < 97){
            mapped_char = shifted_char[(uint8_t)mapped_char];
        }
        keyboard_state.keyboard_buffer[keyboard_state.write_idx] = mapped_char;
        keyboard_state.write_idx = (keyboard_state.write_idx + 1) % 256;
    }
    pic_ack(IRQ_KEYBOARD);
}