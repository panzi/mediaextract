PREFIX=/usr/local
TARGET=posix
INCLUDE=
LIBDIRS=
LIBS=
LDFLAGS=
PLATFORM=posix
BUILDDIR=build-$(TARGET)
OBJ=\
	$(BUILDDIR)/audioextract.o \
	$(BUILDDIR)/audioextract_$(PLATFORM).o \
	$(BUILDDIR)/riff.o \
	$(BUILDDIR)/aiff.o \
	$(BUILDDIR)/ogg.o \
	$(BUILDDIR)/mpg123.o \
	$(BUILDDIR)/mp4.o \
	$(BUILDDIR)/id3.o \
	$(BUILDDIR)/midi.o \
	$(BUILDDIR)/mod.o \
	$(BUILDDIR)/s3m.o \
	$(BUILDDIR)/xm.o \
	$(BUILDDIR)/it.o \
	$(BUILDDIR)/asf.o \
	$(BUILDDIR)/bink.o
CC=gcc
LD=gcc
COMMON_CFLAGS=-Wall -Werror -pedantic -Wextra -std=gnu99 -O2 -fmessage-length=0 -g $(INCLUDE) $(LIBDIRS)
CFLAGS=$(COMMON_CFLAGS)
APPNAME=audioextract
BIN=$(BUILDDIR)/$(APPNAME)

ifeq ($(TARGET),win32)
	PLATFORM=windows
	CC=i686-pc-mingw32-gcc
	LD=i686-pc-mingw32-gcc
	CFLAGS=$(COMMON_CFLAGS) -m32
	LIBS=-lws2_32 -liberty
	APPNAME=audioextract.exe
else
ifeq ($(TARGET),win64)
	PLATFORM=windows
	CC=x86_64-w64-mingw32-gcc
	LD=x86_64-w64-mingw32-gcc
	CFLAGS=$(COMMON_CFLAGS) -m64
	LIBS=-lws2_32 -liberty
	APPNAME=audioextract64.exe
endif
endif

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(OBJ)
	$(LD) $(LIBDIRS) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

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
		src/xm.h \
		src/it.h \
		src/asf.h \
		src/bink.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/audioextract_$(PLATFORM).o: src/audioextract_$(PLATFORM).c src/audioextract.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/riff.o: src/riff.c src/audioextract.h src/riff.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/aiff.o: src/aiff.c src/audioextract.h src/aiff.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/ogg.o: src/ogg.c src/audioextract.h src/ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mpg123.o: src/mpg123.c src/audioextract.h src/mpg123.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mp4.o: src/mp4.c src/audioextract.h src/mp4.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/id3.o: src/id3.c src/audioextract.h src/id3.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/midi.o: src/midi.c src/audioextract.h src/midi.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mod.o: src/mod.c src/audioextract.h src/mod.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/s3m.o: src/s3m.c src/audioextract.h src/s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/xm.o: src/xm.c src/audioextract.h src/xm.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/it.o: src/it.c src/audioextract.h src/it.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/asf.o: src/asf.c src/audioextract.h src/asf.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/bink.o: src/bink.c src/audioextract.h src/bink.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

install: $(PREFIX)/bin/$(APPNAME)

$(PREFIX)/bin/$(APPNAME): $(BIN)
	install -s -D $(BIN) "$@"

uninstall:
	rm -f "$(PREFIX)/bin/$(APPNAME)"

clean:
	rm -f $(BIN) $(OBJ)
