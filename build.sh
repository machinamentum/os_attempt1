SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
export PATH="$SCRIPTPATH/toolchain/bin:$PATH"
nasm -felf32 src/boot.s -o boot.o

INCLUDE_PATHS=-Iinclude
COMMON_FLAGS="$INCLUDE_PATHS -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char"

i686-elf-gcc -c src/main.cpp       -o main.o       $COMMON_FLAGS
i686-elf-gcc -c src/interrupts.cpp -o interrupts.o $COMMON_FLAGS -mgeneral-regs-only
i686-elf-gcc -c src/vga.cpp        -o vga.o        $COMMON_FLAGS
i686-elf-gcc -c src/heap.cpp       -o heap.o       $COMMON_FLAGS

i686-elf-ld -T linker.ld -o myos.bin -O2 -nostdlib boot.o main.o interrupts.o vga.o heap.o

rm *.o
