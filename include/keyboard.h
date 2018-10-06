#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "kernel.h"

struct Input {
	u8 ctrl_pressed;
	u8 shift_pressed;
	u8 action;
    
	u32 keycode;
    
	u8 utf8_code[4];
};

extern Array<Input> keyboard_event_queue;

#define KEY_PRESS        1
#define KEY_RELEASE      2
#define KEY_REPEAT       3

#define KEYCODE_UNDEFINED 0xFFFFFFFF

#define KEYCODE_ESCAPE   27

// The basic representable ASCII characters are 1:1 mapped with keycodes
// however, it is advised to use utf8_code instead of keycode for printing
#define KEYCODE_SPACE    ' '

#define KEYCODE_SINGLE_QUOTE 39

#define KEYCODE_COMMA    ','
#define KEYCODE_MINUS    '-'
#define KEYCODE_PERIOD   '.'
#define KEYCODE_FORWARD_SLASH '/'
#define KEYCODE_0        '0'
#define KEYCODE_1        '1'
#define KEYCODE_2        '2'
#define KEYCODE_3        '3'
#define KEYCODE_4        '4'
#define KEYCODE_5        '5'
#define KEYCODE_6        '6'
#define KEYCODE_7        '7'
#define KEYCODE_8        '8'
#define KEYCODE_9        '9'

#define KEYCODE_SEMICOLON ';'

#define KEYCODE_EQUALS   '='

#define KEYCODE_A        'A'
#define KEYCODE_B        'B'
#define KEYCODE_C        'C'
#define KEYCODE_D        'D'
#define KEYCODE_E        'E'
#define KEYCODE_F        'F'
#define KEYCODE_G        'G'
#define KEYCODE_H        'H'
#define KEYCODE_I        'I'
#define KEYCODE_J        'J'
#define KEYCODE_K        'K'
#define KEYCODE_L        'L'
#define KEYCODE_M        'M'
#define KEYCODE_N        'N'
#define KEYCODE_O        'O'
#define KEYCODE_P        'P'
#define KEYCODE_Q        'Q'
#define KEYCODE_R        'R'
#define KEYCODE_S        'S'
#define KEYCODE_T        'T'
#define KEYCODE_U        'U'
#define KEYCODE_V        'V'
#define KEYCODE_W        'W'
#define KEYCODE_X        'X'
#define KEYCODE_Y        'Y'
#define KEYCODE_Z        'Z'
#define KEYCODE_LEFT_BRACKET '['
#define KEYCODE_BACKSLASH    '\\'
#define KEYCODE_RIGHT_BRACKET ']'
#define KEYCODE_BACKTICK  96

#define KEYCODE_BACKSPACE 127

#define ASCII_EXTENDED_BLOCK 219

#define KEYCODE_F1        256
#define KEYCODE_F2        257
#define KEYCODE_F3        258
#define KEYCODE_F4        259
#define KEYCODE_F5        260
#define KEYCODE_F6        261
#define KEYCODE_F7        262
#define KEYCODE_F8        263
#define KEYCODE_F9        264
#define KEYCODE_F10       265
#define KEYCODE_F11       266
#define KEYCODE_F12       267
#define KEYCODE_TAB       268
#define KEYCODE_LEFT_ALT     269
#define KEYCODE_LEFT_SHIFT   270
#define KEYCODE_LEFT_CONTROL 271
#define KEYCODE_NUMPAD_0     272
#define KEYCODE_NUMPAD_1     273
#define KEYCODE_NUMPAD_2     274
#define KEYCODE_NUMPAD_3     275
#define KEYCODE_NUMPAD_4     276
#define KEYCODE_NUMPAD_5     277
#define KEYCODE_NUMPAD_6     278
#define KEYCODE_NUMPAD_7     279
#define KEYCODE_NUMPAD_8     280
#define KEYCODE_NUMPAD_9     281
#define KEYCODE_NUMPAD_PLUS  282
#define KEYCODE_NUMPAD_ASTERISK 283
#define KEYCODE_NUMPAD_MINUS 284
#define KEYCODE_CAPS_LOCK    285
#define KEYCODE_SCROLL_LOCK  286
#define KEYCODE_RIGHT_SHIFT  287
#define KEYCODE_ENTER        288
#define KEYCODE_NUMBER_LOCK  289
#define KEYCODE_RIGHT_CONTROL 290


#endif
