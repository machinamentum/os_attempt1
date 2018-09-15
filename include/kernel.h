#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdarg.h>

#define ALIGN(x) __attribute__((aligned(x)))
#define PACKED   __attribute__((packed))

#define PAGE_SIZE 0x1000

#define KERNEL_VIRTUAL_BASE_ADDRESS 0xC0000000

#define UNUSED(x) do { (void)(x); } while (0)

#define kNEW(type) (reinterpret_cast<type *>( zero_memory(heap_alloc(sizeof(type)), sizeof(type)) ))

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef float float32;
typedef double float64;

struct String {
    u8 *data;
    s64 length;
};

#define PAGE_PRESENT           (1 << 0)
#define PAGE_READ_WRITE        (1 << 1)
#define PAGE_IS_NOT_PRIVILEGED (1 << 2) // 1; page is accesable to user land, otherwise only the kernel has access
#define PAGE_USE_WRITE_THROUGH (1 << 3) // 1; write-through caching, otherwise write-back
#define PAGE_DO_NOT_CACHE      (1 << 4)
#define PAGE_HAS_BEEN_ACCESSD  (1 << 5)
#define PAGE_IS_DIRTY          (1 << 6)
#define PAGE_SIZE_4MiB         (1 << 7) // 1; page size 4MiB, otherwise page size 4KiB 
#define PAGE_GLOBAL_BIT        (1 << 8)

void map_page_table(u32 *table, u32 virtual_addr);

u32 next_free_page();

u32 virtual_to_physical_address(u32 virtual_addr);

void map_page(u32 physical, u32 virtual_addr, u32 flags);

void unmap_page(u32 virtual_addr);

void memcpy(void *dst, void *src, u32 num);

void *zero_memory(void *dst, u32 size);

s64 strlen(char *c_string);

String temp_string(char *c_string);

void kprint(char *s, ...);

void kerror(char *s, ...);

void kprint(String s, ...);

void kerror(String s, ...);

void _kassert(bool arg, String s, String file, u32 line);

void _kassert(bool arg, char *s, char *file, u32 line);

#define kassert(arg) _kassert((arg), #arg, __FILE__, __LINE__)

extern "C"
void _port_io_write_u32(u16 port, u32 value);
extern "C"
void _port_io_write_u8(u16 port, u8 value);

extern "C"
u8  _port_io_read_u8(u16 port);
extern "C"
u32 _port_io_read_u32(u16 port);
extern "C"
void _io_wait();

void pic_set_eoi(u8 irq);

u16 pic_get_isr();

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

#include "heap.h"

#define For(x) for (auto it : x)

enum ALLOCATOR_MODE {
    FREE,
    ALLOC,
};

typedef void *(*allocator_type)(ALLOCATOR_MODE, void *existing, s64 size);

template <typename T>
struct Array {
    T *data = nullptr;
    s64 count = 0;
    s64 allocated = 0;
    allocator_type allocator = nullptr;

    T &operator [](s64 index) {
        kassert(index >= 0 && index < count);
        return data[index];
    }

    void reserve(s64 size) {
        if (size > allocated) {
        	if (size < 32) size = 32;

            T *ndata;
            if (allocator) ndata = reinterpret_cast<T *>(allocator(ALLOC, nullptr, size * sizeof(T)));
            else ndata = reinterpret_cast<T *>(heap_alloc(size * sizeof(T)));
            memcpy(ndata, data, count * sizeof(T));
            if (allocator) allocator(FREE, data, 0);
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
        if (count) return &data[count-1];
        return nullptr;
    }
};


#endif // KERNEL_H