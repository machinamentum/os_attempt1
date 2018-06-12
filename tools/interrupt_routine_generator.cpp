// used to generate a black set of interrupts routines to be used in interrupts.cpp
// use: gen > output_file.cpp


#include <stdio.h>
#include "../kernel.h"

const char *argument_to_irq_handler = "void *arg";

void write_function(FILE *file, u8 irq_number) {
	fprintf(file, "__attribute__((interrupt))\n");
	fprintf(file, "void __irq_0x%02X_handler(%s) {\n", irq_number, argument_to_irq_handler);
	fprintf(file, "    UNUSED(arg);\n\n");
	if (irq_number >= 0x20 && irq_number <= 0x30) {
		// this is where the PIC IRQs are mapped so inform the PIC that we handled it
		fprintf(file, "    pic_set_eoi(0x%02X);\n", irq_number);
	}
	fprintf(file, "}\n\n");
}

void write_set_idt(FILE *file, u8 irq_number) {
	fprintf(file, "    set_idt_entry(&idt_table[0x%02X], (u32)&__irq_0x%02X_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);\n", irq_number, irq_number);
}

int main(int argc, char **argv) {
	FILE *file = stdout;
	for (short i = 0; i < 256; ++i) {
		write_function(file, i);
	}

	fprintf(file, "void init_interrupt_descriptor_table() {\n");
	for (short i = 0; i < 256; ++i) {
		write_set_idt(file, i);
	}	
	fprintf(file, "}\n\n");
	return 0;
}
