export PATH="$PATH:$HOME/opt/cross/bin"
nasm -felf32 boot.s -o boot.o
i686-elf-gcc -c main.cpp -o kernel.o -std=c++11 -ffreestanding -O2 -Wall -Wextra -Wno-write-strings -funsigned-char
i686-elf-ld -T linker.ld -o myos.bin -O2 -nostdlib boot.o kernel.o 