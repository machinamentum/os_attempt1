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
	mov esp, (stack_top - 0xC0000000)
	add ebx, 0xC0000000
	push ebx

	extern init_page_table_directory
	call init_page_table_directory
	pop ebx
	
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	mov esp, stack_top
	push ebx

	; unmap the first page directory
	; we can't just unmap this in init_page_table_directory because
	; that would make the address that our instruction pointer is at
	; invalid and thus fault!
	extern unmap_page_table
	push 0
	call unmap_page_table
	pop eax

	lea ecx, [start_kernel]
	jmp ecx
	cli
.hang: hlt
	jmp .hang
.end:

start_kernel:
	extern kernel_main
	call kernel_main
	hlt

global invalidate_page_i486
invalidate_page_i486:
	mov eax, [esp+4]
	invlpg [eax]
	ret

global flush_tlb
flush_tlb:
	mov eax, cr3
	mov cr3, eax
	ret

global load_page_directory
load_page_directory:
	mov eax, [esp+4]
	mov cr3, eax
	ret

global enable_paging
enable_paging:
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret

gdtr dw 0
	 dd 0

global set_gdt
set_gdt:
	mov eax, [esp+4]
	mov [gdtr+2], eax
	mov ax, [esp+8]
	mov [gdtr], ax	
	lgdt [gdtr]
	
	jmp 0x08:reload_segments
reload_segments:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret

idtr dw 0
	 dd 0

global set_idt
set_idt:
	mov eax, [esp+4]
	mov [idtr+2], eax
	mov ax, [esp+8]
	mov [idtr], ax
	lidt [idtr]
	ret

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

global _port_io_read_u32
_port_io_read_u32:
	mov dx, [esp+4]
	in eax, dx
	ret

global _io_wait
_io_wait:
	out 0x80, al
	ret


global __irq_0x00_handler
__irq_0x00_handler:
	pushad
	push 0x00
	extern irq_handler	
	call irq_handler
	pop eax
	popad
	iret