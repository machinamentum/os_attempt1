#include "kernel.h"
#include "heap.h"


struct {
    // the default amount of mappable memory due to our heap_page_table in the kernel
    u32 mappable_addr_space = 1024 * 4096;
    u32 mapped_memory = 0;
    u32 watermark = 0;
} heap_info;

void *heap_alloc_no_reserve(u32 size);

void heap_ensure_we_can_map_size(u32 size) {
    u32 memory_left = heap_info.mappable_addr_space - heap_info.mapped_memory;
    while ((memory_left - PAGE_SIZE) < size ) { // arbitrary
        kassert(memory_left >= PAGE_SIZE); // we need at least a page to setup a page table

        // @FIXME we cant have this type of recursion here, I think
        u32 *table = reinterpret_cast<u32 *>(heap_alloc_no_reserve(PAGE_SIZE));
        for (int i = 0; i < 1024; ++i) {
            table[i] = PAGE_READ_WRITE;
        }

        map_page_table(table, HEAP_VIRTUAL_BASE_ADDRESS + heap_info.mappable_addr_space);
        heap_info.mappable_addr_space += 1024 * 4096;
    }
}

u32 heap_mappable_space_left() {
    return (heap_info.mappable_addr_space - heap_info.mapped_memory) - PAGE_SIZE;
}

void init_heap() {
    heap_info.mappable_addr_space = 1024 * 4096;
    heap_info.mapped_memory = 0;
    heap_info.watermark = 0;
}

void *heap_map(u32 size) {
    kprint("Size: %d\n", size);
    kassert((size & (PAGE_SIZE-1)) == 0);
    kassert((heap_info.mappable_addr_space - heap_info.mapped_memory) >= size);

    u32 page_start = HEAP_VIRTUAL_BASE_ADDRESS + heap_info.mapped_memory;
    for (u32 i = 0; i <= size; i += PAGE_SIZE) {
        u32 page = next_free_page();
        map_page(page, HEAP_VIRTUAL_BASE_ADDRESS + heap_info.mapped_memory, PAGE_READ_WRITE);
        heap_info.mapped_memory += PAGE_SIZE;
    }

    return reinterpret_cast<void *>(page_start);
}

void *heap_alloc_no_reserve(u32 size) {
    return heap_map(size);
}

void *heap_reserve(u32 size) {
    heap_ensure_we_can_map_size(size);
    return heap_map(size);
}

void *heap_alloc(u32 size) {
    u32 allocation_size = size;
    if (size & (PAGE_SIZE-1)) {
        size &= ~(PAGE_SIZE-1);
        size += PAGE_SIZE;
    }
    heap_reserve(size);
    
    void *out = reinterpret_cast<void *>(HEAP_VIRTUAL_BASE_ADDRESS + heap_info.watermark);
    heap_info.watermark += allocation_size;
    kassert(heap_info.watermark < heap_info.mapped_memory);
    kprint("heap: Allocated block at %p of size %d\n", out, allocation_size);
    return out;
}