#ifndef HEAP_H
#define HEAP_H

#define HEAP_VIRTUAL_BASE_ADDRESS 0x10000000

void init_heap();
void *heap_alloc(u32 size);

#endif
