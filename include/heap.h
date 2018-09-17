#ifndef HEAP_H
#define HEAP_H

#include "kernel.h"

#define HEAP_VIRTUAL_BASE_ADDRESS 0x10000000

void init_heap();
void *heap_alloc(u32 size);

// @TODO should this return an error code if invalid memory detected or just kassert() ?
void heap_free(void *mem);

void *heap_allocator(ALLOCATOR_MODE mode, void *existing, s64 size);

#endif
