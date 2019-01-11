#! /bin/bash

a=$( echo $1 | sed  "s/\.asm//" ) # strip the file extension .asm

nasm -f elf64 ${a}.asm 

gcc -m64 -static -o  ${a}  ${a}.o -lc

rm ${a}.o

${a} > ${a}.out

rm ${a}