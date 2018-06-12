
#include "kernel.h"
#include "interrupts.h"

extern "C"
void irq_handler(u32 irq) {
    kerror(S("IRQ: %u"), irq);
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
    idt->offset_1 = offset & 0xFFFF;
    idt->selector = 0x08;
    idt->zero = 0;
    idt->type_attr = ((privelege << 5) & 0x3) | type_attr;
    idt->offset_2 = (offset >> 16) & 0xFFFF;
}


__attribute__((interrupt))
void __irq_0x00_handler(void *) {
    irq_handler(0x1);
}

void init_interrupt_descriptor_table() {
    for (int i = 0; i < 256; ++i) {
        Idt_Descriptor *idt = &idt_table[i];
        set_idt_entry(idt, (u32)&__irq_0x00_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    }

    set_idt(&idt_table, sizeof(idt_table[0]) * 256);
}