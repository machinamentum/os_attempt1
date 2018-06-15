nasm -felf32 boot.s -o boot.o
i686-elf-gcc -c main.cpp -o main.o             -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char
i686-elf-gcc -c interrupts.cpp -o interrupts.o -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char -mgeneral-regs-only
i686-elf-gcc -c vga.cpp -o vga.o               -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char

i686-elf-ld -T linker.ld -o myos.bin -O2 -static -nostdlib boot.o main.o interrupts.o vga.o
