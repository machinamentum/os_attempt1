SET TOOLCHAIN=%~dp0\i686-elf-tools\bin\

SET INCLUDE_PATHS=-Iinclude
SET COMMON_FLAGS=%INCLUDE_PATHS% -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char

%TOOLCHAIN%\nasm -felf32    src\boot.s           -o boot.o                                || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\main.cpp         -o main.o         %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\interrupts.cpp   -o interrupts.o   %COMMON_FLAGS% -mgeneral-regs-only    || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\vga.cpp          -o vga.o          %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\heap.cpp         -o heap.o         %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\ide.cpp          -o ide.o          %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\vmware_svga2.cpp -o vmware_svga2.o %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\nuklear.c        -o nuklear.o      %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\math.cpp         -o math.o         %COMMON_FLAGS%         || EXIT /B 1
%TOOLCHAIN%\i686-elf-gcc -c src\print.cpp        -o print.o        %COMMON_FLAGS%         || EXIT /B 1

%TOOLCHAIN%\i686-elf-ld -T linker.ld -o myos.bin -O2 -static -nostdlib boot.o main.o interrupts.o vga.o heap.o ide.o vmware_svga2.o nuklear.o math.o print.o                         || EXIT /B 1

del *.o