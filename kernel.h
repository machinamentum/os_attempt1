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

#endif // KERNEL_H