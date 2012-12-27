BUILDDIR=build
OBJ=$(BUILDDIR)/audioextract.o $(BUILDDIR)/wave.o $(BUILDDIR)/ogg.o $(BUILDDIR)/mpeg.o $(BUILDDIR)/id3.o
CC=gcc
CFLAGS=-Wall -std=c99 -O2 -fmessage-length=0 -g
BIN=$(BUILDDIR)/audioextract

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILDDIR)/audioextract.o: audioextract.c audioextract.h ogg.h wave.h mpeg.h id3.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/wave.o: wave.c audioextract.h wave.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/ogg.o: ogg.c audioextract.h ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mpeg.o: mpeg.c audioextract.h mpeg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/id3.o: id3.c audioextract.h id3.h
	$(CC) $(CFLAGS) $< -o $@ -c

clean:
	rm $(BIN) $(OBJ)
