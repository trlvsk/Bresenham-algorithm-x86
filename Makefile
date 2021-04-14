CC=gcc
ASMBIN=nasm

all: asm cc link
asm:
	$(ASMBIN) -o fun.o -f elf -l fun.lst fun.asm
cc:
	$(CC) -m32 -c -g -O0 -fpack-struct graph_io.c
link:
	$(CC) -m32 -o graph_io graph_io.o fun.o -lm
gdb:
	gdb graph_io
clean:
	rm *.o
	rm graph_io
	rm fun.lst
debug: all gdb