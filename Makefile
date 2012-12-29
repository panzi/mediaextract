PREFIX=/usr/local
BUILDDIR=build
OBJ=\
	$(BUILDDIR)/audioextract.o \
	$(BUILDDIR)/wave.o \
	$(BUILDDIR)/ogg.o \
	$(BUILDDIR)/mpeg.o \
	$(BUILDDIR)/id3.o \
	$(BUILDDIR)/midi.o \
	$(BUILDDIR)/mod.o \
	$(BUILDDIR)/s3m.o
CC=gcc
CFLAGS=-Wall -std=gnu99 -O2 -fmessage-length=0 -g
BIN=$(BUILDDIR)/audioextract

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILDDIR)/audioextract.o: audioextract.c audioextract.h ogg.h wave.h mpeg.h id3.h midi.h mod.h s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/wave.o: wave.c audioextract.h wave.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/ogg.o: ogg.c audioextract.h ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mpeg.o: mpeg.c audioextract.h mpeg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/id3.o: id3.c audioextract.h id3.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/midi.o: midi.c audioextract.h midi.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mod.o: mod.c audioextract.h mod.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/s3m.o: s3m.c audioextract.h s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c

install: all
	install -s -D $(BIN) "$(PREFIX)/bin/audioextract"

uninstall:
	rm -f "$(PREFIX)/bin/audioextract"

clean:
	rm -f $(BIN) $(OBJ)
