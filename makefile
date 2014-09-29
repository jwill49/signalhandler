all: carlo shell

carlo: carlo.c
	gcc -g -o carlo carlo.c

shell: shell.c
	gcc -g -o shell shell.c