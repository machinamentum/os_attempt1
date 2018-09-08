SET TOOLCHAIN=%~dp0\i686-elf-tools\bin\

SET COMMON_FLAGS=-std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char

%TOOLCHAIN%\nasm -felf32 boot.s -o boot.o
%TOOLCHAIN%\i686-elf-gcc -c main.cpp -o main.o             %COMMON_FLAGS%
%TOOLCHAIN%\i686-elf-gcc -c interrupts.cpp -o interrupts.o %COMMON_FLAGS% -mgeneral-regs-only
%TOOLCHAIN%\i686-elf-gcc -c vga.cpp -o vga.o               %COMMON_FLAGS%
%TOOLCHAIN%\i686-elf-gcc -c heap.cpp -o heap.o             %COMMON_FLAGS%

%TOOLCHAIN%\i686-elf-ld -T linker.ld -o myos.bin -O2 -static -nostdlib boot.o main.o interrupts.o vga.o heap.o
