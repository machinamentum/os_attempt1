MBALIGN equ 1 << 0
MEMINFO equ 1 << 1
FLAGS   equ MBALIGN | MEMINFO
MAGIC   equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)


section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM


section .bss
align 16
stack_bottom:
resb 16384
stack_top:


section .text
global _start:function (_start.end - _start)

_start:
	cli
	mov esp, stack_top
	push ebx
	extern kernel_main
	call kernel_main
	cli
.hang: hlt
	jmp .hang
.end:


global _port_io_read_u8
global _port_io_write_u8

global _port_io_write_u32

_port_io_write_u32:
	mov dx, [esp+4]
	mov eax, [esp+8]
	out dx, eax
	ret

_port_io_write_u8:
	mov dx, [esp+4]
	mov al, [esp+8]
	out dx, al
	ret

_port_io_read_u8:
	mov dx, [esp+4]
	in al, dx
	ret