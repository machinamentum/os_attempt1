nasm -felf32 boot.s -o boot.o
D:\LLVM\bin\clang -target i686-none-elf -c main.cpp -o kernel.o -std=c++11 -ffreestanding -O2 -Wall -Wextra 

D:\LLVM\bin\lld -flavor gnu -T linker.ld -o myos.bin -O2 -nostdlib boot.o kernel.o 