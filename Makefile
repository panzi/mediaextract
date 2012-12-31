PREFIX=/usr/local
BUILDDIR=build
OBJ=\
	$(BUILDDIR)/audioextract.o \
	$(BUILDDIR)/wave.o \
	$(BUILDDIR)/ogg.o \
	$(BUILDDIR)/mpg123.o \
	$(BUILDDIR)/mp4.o \
	$(BUILDDIR)/id3.o \
	$(BUILDDIR)/midi.o \
	$(BUILDDIR)/mod.o \
	$(BUILDDIR)/s3m.o \
	$(BUILDDIR)/it.o
CC=gcc
CFLAGS=-Wall -pedantic -Wextra -Werror -std=gnu99 -O2 -fmessage-length=0 -g
BIN=$(BUILDDIR)/audioextract

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILDDIR)/audioextract.o: audioextract.c audioextract.h ogg.h wave.h mpg123.h mp4.h id3.h midi.h mod.h s3m.h it.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/wave.o: wave.c audioextract.h wave.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/ogg.o: ogg.c audioextract.h ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mpg123.o: mpg123.c audioextract.h mpg123.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mp4.o: mp4.c audioextract.h mp4.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/id3.o: id3.c audioextract.h id3.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/midi.o: midi.c audioextract.h midi.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mod.o: mod.c audioextract.h mod.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/s3m.o: s3m.c audioextract.h s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/it.o: it.c audioextract.h it.h
	$(CC) $(CFLAGS) $< -o $@ -c

install: $(PREFIX)/bin/audioextract

$(PREFIX)/bin/audioextract: $(BIN)
	install -s -D $(BIN) "$@"

uninstall:
	rm -f "$(PREFIX)/bin/audioextract"

clean:
	rm -f $(BIN) $(OBJ)
