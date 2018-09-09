#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "kernel.h"

#define INTERRUPT_TYPE_TASK_GATE_32 (0x5)
#define INTERRUPT_TYPE_GATE_16      (0x6)
#define INTERRUPT_TYPE_TRAP_GATE_16 (0x7)
#define INTERRUPT_TYPE_GATE_32      (0xE)
#define INTERRUPT_TYPE_TRAP_GATE_32 (0xF)
#define INTERRUPT_PRESENT           (1 << 7)
#define INTERRUPT_STORAGE_SEGMENT   (1 << 4)

extern "C"
void set_idt(void *idt, u16 size);

void init_interrupt_descriptor_table();

#endif // INTERRUPTS_H
