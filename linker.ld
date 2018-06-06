ENTRY(_start)

SECTIONS
{
	. = 1M;
	.text ALIGN(4096) : ALIGN(4096)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata ALIGN(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}