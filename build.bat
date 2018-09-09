SET TOOLCHAIN=%~dp0\i686-elf-tools\bin\

SET INCLUDE_PATHS=-Iinclude
SET COMMON_FLAGS=%INCLUDE_PATHS% -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char

%TOOLCHAIN%\nasm -felf32    src\boot.s         -o boot.o
%TOOLCHAIN%\i686-elf-gcc -c src\main.cpp       -o main.o       %COMMON_FLAGS%
%TOOLCHAIN%\i686-elf-gcc -c src\interrupts.cpp -o interrupts.o %COMMON_FLAGS% -mgeneral-regs-only
%TOOLCHAIN%\i686-elf-gcc -c src\vga.cpp        -o vga.o        %COMMON_FLAGS%
%TOOLCHAIN%\i686-elf-gcc -c src\heap.cpp       -o heap.o       %COMMON_FLAGS%

%TOOLCHAIN%\i686-elf-ld -T linker.ld -o myos.bin -O2 -static -nostdlib boot.o main.o interrupts.o vga.o heap.o

del *.o