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

#define IRQ_RESULT_CONTINUE 0
#define IRQ_RESULT_HANDLED  1

typedef s32 irq_result_type;
typedef irq_result_type (*irq_handler_type)(s32 irq, void *dev);

void register_irq_handler(s32 irq, String device_name, irq_handler_type handler,  void *dev);
void detach_irq_handler(s32 irq, void *dev);

#endif // INTERRUPTS_H
