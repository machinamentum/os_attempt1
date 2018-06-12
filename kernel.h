#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define ALIGN(x) __attribute__((aligned(x)))
#define PACKED   __attribute__((packed))

#define PAGE_SIZE 0x1000

#define KERNEL_VIRTUAL_BASE_ADDRESS 0xC0000000

#define UNUSED(x) do { (void)(x); } while (0)

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

void memcpy(void *dst, void *src, u32 num);

s64 strlen(char *c_string);

String temp_string(char *c_string);

#define S(str) (temp_string((str)))

void kprint(char *s, ...);

void kerror(char *s, ...);

void kprint(String s, ...);

void kerror(String s, ...);

void _kassert(bool arg, String s, String file, u32 line);

#define kassert(arg) _kassert((arg), S(#arg), S(__FILE__), __LINE__)

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