#! /bin/bash

a=$( echo $1 | sed  "s/\.asm//" ) # strip the file extension .c

nasm -f elf64 ${a}.asm 

# link --> see comment of MichaelPetch below
gcc -m64 -static -o  ${a}  ${a}.o -lc

rm ${a}.o

./${a}