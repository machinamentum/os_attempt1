SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
export PATH="$SCRIPTPATH/toolchain/bin:$PATH"
nasm -felf32 boot.s -o boot.o

COMMON_FLAGS="-std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char"
i686-elf-gcc -c main.cpp       -o main.o       $COMMON_FLAGS
i686-elf-gcc -c interrupts.cpp -o interrupts.o $COMMON_FLAGS -mgeneral-regs-only
i686-elf-gcc -c vga.cpp        -o vga.o        $COMMON_FLAGS
i686-elf-gcc -c heap.cpp       -o heap.o       $COMMON_FLAGS

i686-elf-ld -T linker.ld -o myos.bin -O2 -nostdlib boot.o main.o interrupts.o vga.o
