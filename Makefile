PREFIX=/usr/local
BUILDDIR=build
OBJ=\
	$(BUILDDIR)/audioextract.o \
	$(BUILDDIR)/riff.o \
	$(BUILDDIR)/aiff.o \
	$(BUILDDIR)/ogg.o \
	$(BUILDDIR)/mpg123.o \
	$(BUILDDIR)/mp4.o \
	$(BUILDDIR)/id3.o \
	$(BUILDDIR)/midi.o \
	$(BUILDDIR)/mod.o \
	$(BUILDDIR)/s3m.o \
	$(BUILDDIR)/it.o \
	$(BUILDDIR)/asf.o \
	$(BUILDDIR)/bink.o
CC=gcc
CFLAGS=-Wall -pedantic -Wextra -Werror -std=gnu99 -O2 -fmessage-length=0 -g
BIN=$(BUILDDIR)/audioextract

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(BUILDDIR)/audioextract.o: src/audioextract.c \
		src/audioextract.h \
		src/ogg.h \
		src/riff.h \
		src/aiff.h \
		src/mpg123.h \
		src/mp4.h \
		src/id3.h \
		src/midi.h \
		src/mod.h \
		src/s3m.h \
		src/it.h \
		src/asf.h \
		src/bink.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/riff.o: src/riff.c src/audioextract.h src/riff.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/aiff.o: src/aiff.c src/audioextract.h src/aiff.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/ogg.o: src/ogg.c src/audioextract.h src/ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mpg123.o: src/mpg123.c src/audioextract.h src/mpg123.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mp4.o: src/mp4.c src/audioextract.h src/mp4.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/id3.o: src/id3.c src/audioextract.h src/id3.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/midi.o: src/midi.c src/audioextract.h src/midi.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/mod.o: src/mod.c src/audioextract.h src/mod.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/s3m.o: src/s3m.c src/audioextract.h src/s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/it.o: src/it.c src/audioextract.h src/it.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/asf.o: src/asf.c src/audioextract.h src/asf.h
	$(CC) $(CFLAGS) $< -o $@ -c

$(BUILDDIR)/bink.o: src/bink.c src/audioextract.h src/bink.h
	$(CC) $(CFLAGS) $< -o $@ -c

install: $(PREFIX)/bin/audioextract

$(PREFIX)/bin/audioextract: $(BIN)
	install -s -D $(BIN) "$@"

uninstall:
	rm -f "$(PREFIX)/bin/audioextract"

clean:
	rm -f $(BIN) $(OBJ)
