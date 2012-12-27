.PHONY: all clean

all: oggextract

oggextract: oggextract.c
	gcc -Wall -std=c99 -O2 $< -o $@

clean:
	rm oggextract
