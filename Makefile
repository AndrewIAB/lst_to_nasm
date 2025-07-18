.PHONY: all
all: lst2nasm

lst2nasm: main.c
	gcc $^ -o $@ -O3
