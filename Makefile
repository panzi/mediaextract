.PHONY: all clean

all: audioextract

audioextract: audioextract.c
	gcc -Wall -std=c99 -O2 $< -o $@

clean:
	rm audioextract
