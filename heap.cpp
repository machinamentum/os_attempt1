
#include "heap.h"


struct {
    // the default amount of mappable memory due to our heap_page_table in the kernel
    u32 mappable_addr_space = 1024 * 4096;
    u32 mapped_memory = 0;
} heap_info;

void heap_ensure_we_can_map_size(u32 size) {
    u32 memory_left = heap_info.mappable_addr_space - heap_info.mapped_memory;
    while ((memory_left - PAGE_SIZE) < size ) { // arbitrary
        kassert(memory_left >= PAGE_SIZE); // we need at least a page to setup a page table

        u32 *table = heap_alloc(4096);
        for (int i = 0; i < 1024; ++i) {
            table[i] = PAGE_READ_WRITE;
        }

        map_page_table(table, HEAP_VIRTUAL_BASE_ADDRESS + heap_info.mappable_addr_space);
        heap_info.mappable_addr_space += 1024 * 4096;
    }
}

u32 heap_mappable_space_left() {
    return (heap_info.mappable_addr_space - mapped_memory) - PAGE_SIZE;
}

void init_heap() {
    heap_info.mappable_addr_space = 1024 * 4096;
    heap_info.mapped_memory = 0;
}

bool heap_map(u32 size) {
    kassert((size & (PAGE_SIZE-1)) == 0);
    for (u32 i = 0; i < size; i += PAGE_SIZE) {
        u32 page = next_free_page();
        map_page(page, HEAP_VIRTUAL_BASE_ADDRESS + mapped_memory, PAGE_READ_WRITE);
        heap_info.mapped_memory += PAGE_SIZE;
    }
}

void heap_reserve(u32 size) {
    if (heap_mappable_space_left() > size) return;
    heap_ensure_we_can_map_size(size);
    heap_map(size);
}

void *heap_alloc(u32 size) {
    if (size & (PAGE_SIZE-1)) {
        size &= PAGE_SIZE-1;
        size += PAGE_SIZE;
    }
    heap_reserve(size);

    
}