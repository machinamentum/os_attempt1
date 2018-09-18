
#include "kernel.h"
#include "interrupts.h"

extern "C"
void irq_handler(u32 irq) {
    kerror("IRQ: %u", irq);
}

struct Idt_Descriptor {
    u16 offset_1;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_2;
};

Idt_Descriptor idt_table[256];

void set_idt_entry(Idt_Descriptor *idt, u32 offset, u8 type_attr, u8 privelege) {
	if (offset < 0xC0100000) {
		kerror("OFFSET < 0xC0100000");
	}
    idt->offset_1 = offset & 0xFFFF;
    idt->selector = 0x08;
    idt->zero = 0;
    idt->type_attr = ((privelege << 5) & 0x3) | type_attr;
    idt->offset_2 = (offset >> 16) & 0xFFFF;
}

struct IRQ_Receiver {
    void *dev;
    irq_handler_type handler;
};

Array<IRQ_Receiver> irq_Receiver_table[0x10];

void register_irq_handler(s32 irq, String device_name, irq_handler_type handler,  void *dev) {
    kassert(irq >= 0 && irq <= 0x10);

    For (irq_Receiver_table[irq]) {
        if (it.dev == dev) return; // @TODO return an error code ?
    }

    IRQ_Receiver recv;
    recv.handler = handler;
    recv.dev = dev;

    irq_Receiver_table[irq].add(recv);
}

void detach_irq_handler(s32 irq, void *dev) {

}

static void run_interrupt_handlers(s32 irq) {
    kassert(irq >= 0 && irq <= 0x10);

    Array<IRQ_Receiver> *handlers = &irq_Receiver_table[irq];

    For (*handlers) {
        irq_result_type result = it.handler(irq, it.dev);
        if (result == IRQ_RESULT_HANDLED) return;
        if (result == IRQ_RESULT_CONTINUE) continue;

        // @TODO what should we do if the driver returns other codes?
    }
}

__attribute__((interrupt))
void __irq_0x00_handler(void *arg) {
    UNUSED(arg);
    kerror("DIVIDE BY ZERO!");
}

__attribute__((interrupt))
void __irq_0x01_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x02_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x03_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x04_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x05_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x06_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x07_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x08_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x09_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x0A_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x0B_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x0C_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x0D_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x0E_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    kerror("PAGE FAULT: %u", error_code);
}

__attribute__((interrupt))
void __irq_0x0F_handler(void *arg) {
    UNUSED(arg);
}

__attribute__((interrupt))
void __irq_0x10_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x11_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x12_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x13_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x14_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x15_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x16_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x17_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x18_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x19_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x1A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x1B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x1C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x1D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x1E_handler(void *arg, u32 error_code) {
    UNUSED(arg);
    UNUSED(error_code);
}

__attribute__((interrupt))
void __irq_0x1F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x20_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(0);
    pic_set_eoi(0x20);
}

#include "keyboard.h"

u32 scancode_set2_table[] = {
    KEYCODE_UNDEFINED,
    KEYCODE_F9,
    KEYCODE_UNDEFINED,
    KEYCODE_F5,
    KEYCODE_F3,
    KEYCODE_F1,
    KEYCODE_F2,
    KEYCODE_F12,
    KEYCODE_UNDEFINED,
    KEYCODE_F10,
    KEYCODE_F8,
    KEYCODE_F6,
    KEYCODE_F4,
    KEYCODE_TAB,
    KEYCODE_BACKTICK,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_LEFT_ALT,
    KEYCODE_LEFT_SHIFT,
    KEYCODE_UNDEFINED,
    KEYCODE_LEFT_CONTROL,
    KEYCODE_Q,
    KEYCODE_1,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_Z,
    KEYCODE_S,
    KEYCODE_A,
    KEYCODE_W,
    KEYCODE_2,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_C,
    KEYCODE_X,
    KEYCODE_D,
    KEYCODE_E,
    KEYCODE_4,
    KEYCODE_3,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_SPACE,
    KEYCODE_V,
    KEYCODE_F,
    KEYCODE_T,
    KEYCODE_R,
    KEYCODE_5,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_N,
    KEYCODE_B,
    KEYCODE_H,
    KEYCODE_G,
    KEYCODE_Y,
    KEYCODE_6,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_M,
    KEYCODE_J,
    KEYCODE_U,
    KEYCODE_7,
    KEYCODE_8,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_COMMA,
    KEYCODE_K,
    KEYCODE_I,
    KEYCODE_O,
    KEYCODE_0,
    KEYCODE_9,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_PERIOD,
    KEYCODE_FORWARD_SLASH,
    KEYCODE_L,
    KEYCODE_SEMICOLON,
    KEYCODE_P,
    KEYCODE_MINUS,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_SINGLE_QUOTE,
    KEYCODE_UNDEFINED,
    KEYCODE_LEFT_BRACKET,
    KEYCODE_EQUALS,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_CAPS_LOCK,
    KEYCODE_RIGHT_SHIFT,
    KEYCODE_ENTER,
    KEYCODE_RIGHT_BRACKET,
    KEYCODE_UNDEFINED,
    KEYCODE_BACKSLASH,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_BACKSPACE,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_NUMPAD_1,
    KEYCODE_UNDEFINED,
    KEYCODE_NUMPAD_4,
    KEYCODE_NUMPAD_7,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_NUMPAD_0,
    KEYCODE_PERIOD,
    KEYCODE_NUMPAD_2,
    KEYCODE_NUMPAD_5,
    KEYCODE_NUMPAD_6,
    KEYCODE_NUMPAD_8,
    KEYCODE_ESCAPE,
    KEYCODE_NUMBER_LOCK,
    KEYCODE_F11,
    KEYCODE_NUMPAD_PLUS,
    KEYCODE_NUMPAD_3,
    KEYCODE_NUMPAD_MINUS,
    KEYCODE_NUMPAD_ASTERISK,
    KEYCODE_NUMPAD_9,
    KEYCODE_SCROLL_LOCK,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_UNDEFINED,
    KEYCODE_F7,
};

u8 _shift_pressed = 0;
u8 _ctrl_pressed = 0;

Array<Input> keyboard_event_queue;

u8 get_ascii_representable_character(u32 keycode, bool shift_pressed) {
    if (keycode >= 0x20 && keycode < 0x7F) {
        if (keycode >= KEYCODE_A && keycode <= KEYCODE_Z) {
            if (!shift_pressed) return static_cast<u8>(keycode + 0x20);
        }

        if (keycode >= KEYCODE_0 && keycode <= KEYCODE_9) {
            u8 value = static_cast<u8>(keycode - KEYCODE_0);
            if (shift_pressed) return ")!@#$%^&*("[value];
        }

        return static_cast<u8>(keycode);
    }

    if (keycode >= KEYCODE_NUMPAD_0 && keycode <= KEYCODE_NUMPAD_9) {
        u8 value = static_cast<u8>(keycode - KEYCODE_NUMPAD_0);
        return KEYCODE_0 + value;
    }

    if (keycode == KEYCODE_ENTER) return '\n';

    return ASCII_EXTENDED_BLOCK;
}

__attribute__((interrupt))
void __irq_0x21_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(1);

    if (_port_io_read_u8(PS2_STATUS) & PS2_STATUS_OUTPUT_BUFFER_BIT) {
        u8 scancode = _port_io_read_u8(PS2_DATA); _io_wait();

        u8 action = KEY_PRESS;
        if (scancode == 0xF0) {
            action = KEY_RELEASE;
            // @TODO maybe wait here?
            scancode = _port_io_read_u8(PS2_DATA); _io_wait();
        } else if (scancode == 0xE0) {
            kassert(false);
        }

        u8 temp = _port_io_read_u8(0x61);
        _port_io_write_u8(0x61, temp | 0x80); _io_wait();
        _port_io_write_u8(0x61, temp); _io_wait();

        if (scancode >= 0x84) kprint("SCANCODE: %X\n", scancode);
        else {
            u32 keycode = scancode_set2_table[scancode];
            if (keycode == KEYCODE_LEFT_CONTROL || keycode == KEYCODE_RIGHT_CONTROL) {
                _ctrl_pressed = action;
            } else if (keycode == KEYCODE_LEFT_SHIFT || keycode == KEYCODE_RIGHT_SHIFT) {
                _shift_pressed = action;
            } else {
                u8 ascii = get_ascii_representable_character(keycode, _shift_pressed == KEY_PRESS);
                // kprint("KEYBOARD: action: %d, char: %c\n", action, ascii);
                // if (buffer_count < 255) buffer[buffer_count++] = ascii;
                {
                    Input i;
                    i.shift_pressed = _shift_pressed;
                    i.ctrl_pressed = _ctrl_pressed;
                    i.action = action;
                    i.keycode = keycode;
                    i.utf8_code[0] = ascii; // @Hack @FixMe

                    keyboard_event_queue.add(i);
                }
            }
        }
    }

    pic_set_eoi(0x21);
}

__attribute__((interrupt))
void __irq_0x22_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(2);
    pic_set_eoi(0x22);
}

__attribute__((interrupt))
void __irq_0x23_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(3);
    pic_set_eoi(0x23);
}

__attribute__((interrupt))
void __irq_0x24_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(4);
    pic_set_eoi(0x24);
}

__attribute__((interrupt))
void __irq_0x25_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(5);
    pic_set_eoi(0x25);
}

__attribute__((interrupt))
void __irq_0x26_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(6);
    pic_set_eoi(0x26);
}

__attribute__((interrupt))
void __irq_0x27_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(7);
    pic_set_eoi(0x27);
}

__attribute__((interrupt))
void __irq_0x28_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(8);
    pic_set_eoi(0x28);
}

__attribute__((interrupt))
void __irq_0x29_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(9);
    pic_set_eoi(0x29);
}

__attribute__((interrupt))
void __irq_0x2A_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(10);
    pic_set_eoi(0x2A);
}

__attribute__((interrupt))
void __irq_0x2B_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(11);
    pic_set_eoi(0x2B);
}

__attribute__((interrupt))
void __irq_0x2C_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(12);
    pic_set_eoi(0x2C);
}

__attribute__((interrupt))
void __irq_0x2D_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(13);
    pic_set_eoi(0x2D);
}

__attribute__((interrupt))
void __irq_0x2E_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(14);
    pic_set_eoi(0x2E);
}

__attribute__((interrupt))
void __irq_0x2F_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(15);
    pic_set_eoi(0x2F);
}

__attribute__((interrupt))
void __irq_0x30_handler(void *arg) {
    UNUSED(arg);
    run_interrupt_handlers(16);
    pic_set_eoi(0x30);
}

__attribute__((interrupt))
void __irq_0x31_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x32_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x33_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x34_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x35_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x36_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x37_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x38_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x39_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x3F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x40_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x41_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x42_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x43_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x44_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x45_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x46_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x47_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x48_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x49_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x4F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x50_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x51_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x52_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x53_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x54_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x55_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x56_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x57_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x58_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x59_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x5F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x60_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x61_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x62_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x63_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x64_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x65_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x66_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x67_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x68_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x69_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x6F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x70_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x71_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x72_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x73_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x74_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x75_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x76_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x77_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x78_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x79_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x7F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x80_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x81_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x82_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x83_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x84_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x85_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x86_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x87_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x88_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x89_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x8F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x90_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x91_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x92_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x93_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x94_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x95_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x96_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x97_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x98_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x99_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9A_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9B_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9C_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9D_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9E_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0x9F_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xA9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAD_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xAF_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xB9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBD_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xBF_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xC9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCD_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xCF_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xD9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDD_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xDF_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xE9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xEA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xEB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xEC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xED_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xEE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xEF_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF0_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF1_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF2_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF3_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF4_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF5_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF6_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF7_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF8_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xF9_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFA_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFB_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFC_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFD_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFE_handler(void *arg) {
    UNUSED(arg);

}

__attribute__((interrupt))
void __irq_0xFF_handler(void *arg) {
    UNUSED(arg);

}

void init_interrupt_descriptor_table() {
    set_idt_entry(&idt_table[0x00], (u32)&__irq_0x00_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x01], (u32)&__irq_0x01_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x02], (u32)&__irq_0x02_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x03], (u32)&__irq_0x03_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x04], (u32)&__irq_0x04_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x05], (u32)&__irq_0x05_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x06], (u32)&__irq_0x06_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x07], (u32)&__irq_0x07_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x08], (u32)&__irq_0x08_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x09], (u32)&__irq_0x09_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0A], (u32)&__irq_0x0A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0B], (u32)&__irq_0x0B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0C], (u32)&__irq_0x0C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0D], (u32)&__irq_0x0D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0E], (u32)&__irq_0x0E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x0F], (u32)&__irq_0x0F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x10], (u32)&__irq_0x10_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x11], (u32)&__irq_0x11_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x12], (u32)&__irq_0x12_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x13], (u32)&__irq_0x13_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x14], (u32)&__irq_0x14_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x15], (u32)&__irq_0x15_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x16], (u32)&__irq_0x16_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x17], (u32)&__irq_0x17_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x18], (u32)&__irq_0x18_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x19], (u32)&__irq_0x19_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1A], (u32)&__irq_0x1A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1B], (u32)&__irq_0x1B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1C], (u32)&__irq_0x1C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1D], (u32)&__irq_0x1D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1E], (u32)&__irq_0x1E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x1F], (u32)&__irq_0x1F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x20], (u32)&__irq_0x20_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x21], (u32)&__irq_0x21_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x22], (u32)&__irq_0x22_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x23], (u32)&__irq_0x23_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x24], (u32)&__irq_0x24_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x25], (u32)&__irq_0x25_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x26], (u32)&__irq_0x26_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x27], (u32)&__irq_0x27_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x28], (u32)&__irq_0x28_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x29], (u32)&__irq_0x29_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2A], (u32)&__irq_0x2A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2B], (u32)&__irq_0x2B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2C], (u32)&__irq_0x2C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2D], (u32)&__irq_0x2D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2E], (u32)&__irq_0x2E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x2F], (u32)&__irq_0x2F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x30], (u32)&__irq_0x30_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x31], (u32)&__irq_0x31_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x32], (u32)&__irq_0x32_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x33], (u32)&__irq_0x33_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x34], (u32)&__irq_0x34_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x35], (u32)&__irq_0x35_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x36], (u32)&__irq_0x36_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x37], (u32)&__irq_0x37_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x38], (u32)&__irq_0x38_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x39], (u32)&__irq_0x39_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3A], (u32)&__irq_0x3A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3B], (u32)&__irq_0x3B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3C], (u32)&__irq_0x3C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3D], (u32)&__irq_0x3D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3E], (u32)&__irq_0x3E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x3F], (u32)&__irq_0x3F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x40], (u32)&__irq_0x40_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x41], (u32)&__irq_0x41_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x42], (u32)&__irq_0x42_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x43], (u32)&__irq_0x43_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x44], (u32)&__irq_0x44_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x45], (u32)&__irq_0x45_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x46], (u32)&__irq_0x46_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x47], (u32)&__irq_0x47_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x48], (u32)&__irq_0x48_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x49], (u32)&__irq_0x49_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4A], (u32)&__irq_0x4A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4B], (u32)&__irq_0x4B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4C], (u32)&__irq_0x4C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4D], (u32)&__irq_0x4D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4E], (u32)&__irq_0x4E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x4F], (u32)&__irq_0x4F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x50], (u32)&__irq_0x50_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x51], (u32)&__irq_0x51_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x52], (u32)&__irq_0x52_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x53], (u32)&__irq_0x53_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x54], (u32)&__irq_0x54_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x55], (u32)&__irq_0x55_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x56], (u32)&__irq_0x56_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x57], (u32)&__irq_0x57_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x58], (u32)&__irq_0x58_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x59], (u32)&__irq_0x59_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5A], (u32)&__irq_0x5A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5B], (u32)&__irq_0x5B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5C], (u32)&__irq_0x5C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5D], (u32)&__irq_0x5D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5E], (u32)&__irq_0x5E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x5F], (u32)&__irq_0x5F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x60], (u32)&__irq_0x60_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x61], (u32)&__irq_0x61_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x62], (u32)&__irq_0x62_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x63], (u32)&__irq_0x63_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x64], (u32)&__irq_0x64_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x65], (u32)&__irq_0x65_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x66], (u32)&__irq_0x66_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x67], (u32)&__irq_0x67_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x68], (u32)&__irq_0x68_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x69], (u32)&__irq_0x69_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6A], (u32)&__irq_0x6A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6B], (u32)&__irq_0x6B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6C], (u32)&__irq_0x6C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6D], (u32)&__irq_0x6D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6E], (u32)&__irq_0x6E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x6F], (u32)&__irq_0x6F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x70], (u32)&__irq_0x70_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x71], (u32)&__irq_0x71_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x72], (u32)&__irq_0x72_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x73], (u32)&__irq_0x73_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x74], (u32)&__irq_0x74_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x75], (u32)&__irq_0x75_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x76], (u32)&__irq_0x76_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x77], (u32)&__irq_0x77_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x78], (u32)&__irq_0x78_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x79], (u32)&__irq_0x79_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7A], (u32)&__irq_0x7A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7B], (u32)&__irq_0x7B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7C], (u32)&__irq_0x7C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7D], (u32)&__irq_0x7D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7E], (u32)&__irq_0x7E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x7F], (u32)&__irq_0x7F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x80], (u32)&__irq_0x80_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x81], (u32)&__irq_0x81_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x82], (u32)&__irq_0x82_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x83], (u32)&__irq_0x83_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x84], (u32)&__irq_0x84_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x85], (u32)&__irq_0x85_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x86], (u32)&__irq_0x86_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x87], (u32)&__irq_0x87_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x88], (u32)&__irq_0x88_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x89], (u32)&__irq_0x89_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8A], (u32)&__irq_0x8A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8B], (u32)&__irq_0x8B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8C], (u32)&__irq_0x8C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8D], (u32)&__irq_0x8D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8E], (u32)&__irq_0x8E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x8F], (u32)&__irq_0x8F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x90], (u32)&__irq_0x90_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x91], (u32)&__irq_0x91_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x92], (u32)&__irq_0x92_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x93], (u32)&__irq_0x93_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x94], (u32)&__irq_0x94_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x95], (u32)&__irq_0x95_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x96], (u32)&__irq_0x96_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x97], (u32)&__irq_0x97_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x98], (u32)&__irq_0x98_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x99], (u32)&__irq_0x99_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9A], (u32)&__irq_0x9A_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9B], (u32)&__irq_0x9B_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9C], (u32)&__irq_0x9C_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9D], (u32)&__irq_0x9D_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9E], (u32)&__irq_0x9E_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0x9F], (u32)&__irq_0x9F_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA0], (u32)&__irq_0xA0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA1], (u32)&__irq_0xA1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA2], (u32)&__irq_0xA2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA3], (u32)&__irq_0xA3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA4], (u32)&__irq_0xA4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA5], (u32)&__irq_0xA5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA6], (u32)&__irq_0xA6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA7], (u32)&__irq_0xA7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA8], (u32)&__irq_0xA8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xA9], (u32)&__irq_0xA9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAA], (u32)&__irq_0xAA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAB], (u32)&__irq_0xAB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAC], (u32)&__irq_0xAC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAD], (u32)&__irq_0xAD_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAE], (u32)&__irq_0xAE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xAF], (u32)&__irq_0xAF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB0], (u32)&__irq_0xB0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB1], (u32)&__irq_0xB1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB2], (u32)&__irq_0xB2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB3], (u32)&__irq_0xB3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB4], (u32)&__irq_0xB4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB5], (u32)&__irq_0xB5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB6], (u32)&__irq_0xB6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB7], (u32)&__irq_0xB7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB8], (u32)&__irq_0xB8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xB9], (u32)&__irq_0xB9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBA], (u32)&__irq_0xBA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBB], (u32)&__irq_0xBB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBC], (u32)&__irq_0xBC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBD], (u32)&__irq_0xBD_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBE], (u32)&__irq_0xBE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xBF], (u32)&__irq_0xBF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC0], (u32)&__irq_0xC0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC1], (u32)&__irq_0xC1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC2], (u32)&__irq_0xC2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC3], (u32)&__irq_0xC3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC4], (u32)&__irq_0xC4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC5], (u32)&__irq_0xC5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC6], (u32)&__irq_0xC6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC7], (u32)&__irq_0xC7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC8], (u32)&__irq_0xC8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xC9], (u32)&__irq_0xC9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCA], (u32)&__irq_0xCA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCB], (u32)&__irq_0xCB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCC], (u32)&__irq_0xCC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCD], (u32)&__irq_0xCD_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCE], (u32)&__irq_0xCE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xCF], (u32)&__irq_0xCF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD0], (u32)&__irq_0xD0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD1], (u32)&__irq_0xD1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD2], (u32)&__irq_0xD2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD3], (u32)&__irq_0xD3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD4], (u32)&__irq_0xD4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD5], (u32)&__irq_0xD5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD6], (u32)&__irq_0xD6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD7], (u32)&__irq_0xD7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD8], (u32)&__irq_0xD8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xD9], (u32)&__irq_0xD9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDA], (u32)&__irq_0xDA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDB], (u32)&__irq_0xDB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDC], (u32)&__irq_0xDC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDD], (u32)&__irq_0xDD_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDE], (u32)&__irq_0xDE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xDF], (u32)&__irq_0xDF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE0], (u32)&__irq_0xE0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE1], (u32)&__irq_0xE1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE2], (u32)&__irq_0xE2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE3], (u32)&__irq_0xE3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE4], (u32)&__irq_0xE4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE5], (u32)&__irq_0xE5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE6], (u32)&__irq_0xE6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE7], (u32)&__irq_0xE7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE8], (u32)&__irq_0xE8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xE9], (u32)&__irq_0xE9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xEA], (u32)&__irq_0xEA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xEB], (u32)&__irq_0xEB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xEC], (u32)&__irq_0xEC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xED], (u32)&__irq_0xED_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xEE], (u32)&__irq_0xEE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xEF], (u32)&__irq_0xEF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF0], (u32)&__irq_0xF0_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF1], (u32)&__irq_0xF1_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF2], (u32)&__irq_0xF2_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF3], (u32)&__irq_0xF3_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF4], (u32)&__irq_0xF4_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF5], (u32)&__irq_0xF5_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF6], (u32)&__irq_0xF6_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF7], (u32)&__irq_0xF7_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF8], (u32)&__irq_0xF8_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xF9], (u32)&__irq_0xF9_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFA], (u32)&__irq_0xFA_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFB], (u32)&__irq_0xFB_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFC], (u32)&__irq_0xFC_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFD], (u32)&__irq_0xFD_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFE], (u32)&__irq_0xFE_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    set_idt_entry(&idt_table[0xFF], (u32)&__irq_0xFF_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);

    set_idt(&idt_table, sizeof(idt_table[0]) * 256);
}
