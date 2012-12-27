BUILDDIR=build
OBJ=$(BUILDDIR)/audioextract.o $(BUILDDIR)/wave.o $(BUILDDIR)/ogg.o
CC=gcc
CFLAGS=-Wall -std=c99 -O2 -fmessage-length=0 -g
BIN=$(BUILDDIR)/audioextract

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILDDIR)/audioextract.o: audioextract.c audioextract.h ogg.h wave.h $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/wave.o: wave.c audioextract.h wave.h $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/ogg.o: ogg.c audioextract.h ogg.h $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm $(BIN) $(OBJ)
