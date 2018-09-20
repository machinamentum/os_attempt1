

#include "kernel.h"
#include "pci.h"
#include "driver_interface.h"

#define SVGA_MAGIC (0x900000UL << 8)
#define SVGA_ID_0 (SVGA_MAGIC)
#define SVGA_ID_1 (SVGA_MAGIC | 1)
#define SVGA_ID_2 (SVGA_MAGIC | 2)

#define SVGA_REG_ID          0
#define SVGA_REG_ENABLE      1
#define SVGA_REG_WIDTH       2
#define SVGA_REG_HEIGHT      3
#define SVGA_REG_MAX_WIDTH   4
#define SVGA_REG_MAX_HEIGHT  5
#define SVGA_REG_DEPTH       6
#define SVGA_REG_BITS_PER_PIXEL 7
#define SVGA_REG_PSEUDOCOLOR 8
#define SVGA_REG_RED_MASK    9
#define SVGA_REG_GREEN_MASK  10
#define SVGA_REG_BLUE_MASK   11
#define SVGA_REG_BYTES_PER_LINE 12
#define SVGA_REG_FB_OFFSET   13
#define SVGA_REG_VRAM_SIZE   14
#define SVGA_REG_FB_SIZE     15

#define SVGA_REG_CONFIG_DONE 20


struct VMW_SVGA_Driver {
	u16 index_port;
	u16 value_port;
	u16 bios_port;
	u16 irqstatus_port;
} svga_driver;

u32 svga_read_reg(VMW_SVGA_Driver *svga, u32 reg) {
	_port_io_write_u32(svga->index_port, reg);
	return _port_io_read_u32(svga->value_port);
}

void svga_write_reg(VMW_SVGA_Driver *svga, u32 reg, u32 value) {
	_port_io_write_u32(svga->index_port, reg);
	_port_io_write_u32(svga->value_port, value);
}

void svga_set_enable(VMW_SVGA_Driver *svga, u32 val) {
	svga_write_reg(svga, SVGA_REG_ENABLE, 1);
	svga_write_reg(svga, SVGA_REG_CONFIG_DONE, 1);

	u32 en = svga_read_reg(svga, SVGA_REG_ENABLE);
	kprint("EN: %u\n", en);
}

void svga_set_mode(VMW_SVGA_Driver *svga, u32 width, u32 height, u32 bpp) {
	svga_write_reg(svga, SVGA_REG_WIDTH, width);
	svga_write_reg(svga, SVGA_REG_HEIGHT, height);
	svga_write_reg(svga, SVGA_REG_BITS_PER_PIXEL, bpp);

	svga_read_reg(svga, SVGA_REG_WIDTH);
	svga_read_reg(svga, SVGA_REG_HEIGHT);
	svga_read_reg(svga, SVGA_REG_BITS_PER_PIXEL);

	svga_read_reg(svga, SVGA_REG_BYTES_PER_LINE);
	svga_read_reg(svga, SVGA_REG_DEPTH);
	svga_read_reg(svga, SVGA_REG_PSEUDOCOLOR);
	svga_read_reg(svga, SVGA_REG_RED_MASK);
	svga_read_reg(svga, SVGA_REG_GREEN_MASK);
	svga_read_reg(svga, SVGA_REG_BLUE_MASK);
}

void create_svga_driver(Pci_Device_Config *header) {
    kassert( (header->header_type & (~PCI_HEADER_MULTIFUNCTION_BIT)) == 0);

    pci_enable_memory(header);

    kprint("BAR0: %X\n", header->type_00.bar0);
    kprint("BAR1: %X\n", header->type_00.bar1);
    kprint("BAR2: %X\n", header->type_00.bar2);
    kprint("BAR3: %X\n", header->type_00.bar3);
    kprint("BAR4: %X\n", header->type_00.bar4);
    kprint("BAR5: %X\n", header->type_00.bar5);
    kprint("ProgIF: %X\n", header->prog_if);

    VMW_SVGA_Driver *svga = &svga_driver;
    u16 bar0 = static_cast<u16>(header->type_00.bar0 & (~0x3));
    svga->index_port = bar0 + 0;
    svga->value_port = bar0 + 1;
    svga->bios_port  = bar0 + 2;
    svga->irqstatus_port = bar0 + 8;

    u32 target_version = SVGA_ID_2;
    for (u32 id = target_version; id >= SVGA_ID_0; id--) {
    	svga_write_reg(svga, SVGA_REG_ID, SVGA_ID_0);
    	u32 val = svga_read_reg(svga, SVGA_REG_ID);
    	kprint("val: %X\n", val);

    	if (val == id) {
    		target_version = val;
    		break;
    	}
	}



	u32 fb_size = svga_read_reg(svga, SVGA_REG_FB_SIZE);
	u32 fb_start = header->type_00.bar1 & (~0xf);

	// u32 mem_size = svga_read_reg(svga, svga_REG_MEM);
	// u32 mem_start = header->type_00.bar2 & (~0x3);

	u32 *table = reinterpret_cast<u32 *>(heap_alloc(PAGE_SIZE));
    for (int i = 0; i < 1024; ++i) {
        table[i] = PAGE_READ_WRITE;
    }

    map_page_table(table, DRIVER_SAFE_USERLAND_VIRTUAL_ADDRESS);

    for (u32 phys = fb_start; phys < fb_size; phys += PAGE_SIZE) {
		map_page(phys, DRIVER_SAFE_USERLAND_VIRTUAL_ADDRESS, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_DO_NOT_CACHE);
	}

	u32 max_width = svga_read_reg(svga, SVGA_REG_MAX_WIDTH);
	u32 max_height = svga_read_reg(svga, SVGA_REG_MAX_HEIGHT);
	u32 bpp = svga_read_reg(svga, SVGA_REG_BITS_PER_PIXEL);

	kprint("MODE: %u, %u, %u\n", max_width, max_height, bpp);

	u32 width = svga_read_reg(svga, SVGA_REG_WIDTH);
	u32 height = svga_read_reg(svga, SVGA_REG_HEIGHT);

	kprint("MODE: %u, %u, %u\n", width, height, bpp);

	svga_set_mode(svga, 1280, 720, bpp);

	svga_set_enable(svga, 1);

	width = svga_read_reg(svga, SVGA_REG_WIDTH);
	height = svga_read_reg(svga, SVGA_REG_HEIGHT);

	kprint("MODE: %u, %u, %u\n", width, height, bpp);

	u32 offset = svga_read_reg(svga, SVGA_REG_FB_OFFSET);

	u32 *fb = reinterpret_cast<u32 *>(DRIVER_SAFE_USERLAND_VIRTUAL_ADDRESS + offset);
	for (u32 i = offset; i < fb_size; i += 4) {
		fb[i] = 0xFFFFFFFF;
	}

	for (;;) asm("hlt");

}