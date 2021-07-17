OUTPUT=main

all:
	gcc -o $(OUTPUT) -std=c11 -g -m32 -Wall -Wextra -DOUTPUT=\"$(OUTPUT)\" main.c
