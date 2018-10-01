#ifndef KERNEL_H
#define KERNEL_H

#include "kernel_c.h"

#include <stdarg.h>

#define ALIGN(x) __attribute__((aligned(x)))
#define PACKED   __attribute__((packed))

#define PAGE_SIZE 0x1000

#define KERNEL_VIRTUAL_BASE_ADDRESS 0xC0000000

#define UNUSED(x) do { (void)(x); } while (0)

#define kNEW(type) (reinterpret_cast<type *>( zero_memory(heap_alloc(sizeof(type)), sizeof(type)) ))

#include "string.h"

void _kassert(bool arg, String s, String file, u32 line);

void kprint(String s, ...);

void kerror(String s, ...);

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_COMMAND 0x64

#define PS2_STATUS_OUTPUT_BUFFER_BIT (1 << 0)
#define PS2_STATUS_INPUT_BUFFER_BIT  (1 << 1)
#define PS2_STATUS_SYSTEM_FLAG_BIT   (1 << 2)
#define PS2_STATUS_COMMAND_DATA_BIT  (1 << 3)
#define PS2_STATUS_TIMEOUT_ERROR_BIT (1 << 6)
#define PS2_STATUS_PARITY_ERROR_BIT  (1 << 7)

#define PS2_INTERNAL_RAM_SIZE 0x20
#define PS2_CMD_READ_BYTE0     0x20
#define PS2_CMD_WRITE_BYTE0    0x60
#define PS2_CMD_PORT_2_DISABLE 0xA7
#define PS2_CMD_PORT_2_ENABLE  0xA8
#define PS2_CMD_PORT_2_TEST    0xA9
#define PS2_CMD_CONTROLLER_TEST 0xAA

#define PS2_CMD_PORT_1_DISABLE 0xAD
#define PS2_CMD_PORT_1_ENABLE  0xAE
#define PS2_CMD_PORT_1_TEST    0xAB

#define PS2_CMD_READ_OUTPUT_PORT  0xD0
#define PS2_CMD_WRITE_OUTPUT_PORT 0xD1

#define PS2_CONFIG_PORT_1_INTERRUPT_BIT   (1 << 0)
#define PS2_CONFIG_PORT_2_INTERRUPT_BIT   (1 << 1)
#define PS2_CONFIG_SYSTEM_FLAG_BIT        (1 << 2)
#define PS2_CONFIG_PORT_1_CLOCK_BIT       (1 << 4)
#define PS2_CONFIG_PORT_2_CLOCK_BIT       (1 << 5)
#define PS2_CONFIG_PORT_1_TRANSLATION_BIT (1 << 6)

void ps2_wait_for_response();
void ps2_wait_for_output_clear();
void ps2_wait_for_input_ready();

enum ALLOCATOR_MODE {
    ALLOCATOR_MODE_FREE,
    ALLOCATOR_MODE_ALLOC,
};

typedef void *(*allocator_type)(ALLOCATOR_MODE, void *existing, s64 size);

#include "heap.h"

#define For(x) for (auto it : (x) )

template <typename T>
struct Array {
    T *data = nullptr;
    s64 count = 0;
    s64 allocated = 0;
    allocator_type allocator = heap_allocator;
    
    T &operator [](s64 index) {
        kassert(index >= 0 && index < count);
        return data[index];
    }
    
    void reserve(s64 size) {
        kassert(allocator);
        
        if (size > allocated) {
        	if (size < 32) size = 32;
            
            kassert(allocator == heap_allocator);
            
            T *ndata;
            ndata = reinterpret_cast<T *>(allocator(ALLOCATOR_MODE_ALLOC, nullptr, size * sizeof(T)));
            memcpy(ndata, data, count * sizeof(T));
            if (data) allocator(ALLOCATOR_MODE_FREE, data, 0);
            data = ndata;
            allocated = size;
        }
    }
    
    void resize(s64 size) {
        reserve(size);
        
        count = size;
    }
    
    void add(T item) {
    	resize(count+1);
    	data[count-1] = item;
    }
    
    void clear() {
    	count = 0;
    }
    
    T *begin() {
        if (count) return &data[0];
        return nullptr;
    }
    
    T *end() {
        if (count) return &data[count];
        return nullptr;
    }
};


#endif // KERNEL_H