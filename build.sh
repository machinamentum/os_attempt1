nasm -felf32 boot.s -o boot.o
clang -target i686-none-elf -c main.cpp -o kernel.o -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-writable-strings
lld -flavor gnu -T linker.ld -o myos.bin -O2 -nostdlib boot.o kernel.o 