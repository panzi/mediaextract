PREFIX=/usr/local
TARGET=$(shell uname|tr '[A-Z]' '[a-z]')$(shell getconf LONG_BIT)
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
	$(BUILDDIR)/bink.o \
	$(BUILDDIR)/au.o \
	$(BUILDDIR)/smk.o \
	$(BUILDDIR)/bmp.o \
	$(BUILDDIR)/png.o
CC=gcc
LD=$(CC)
COMMON_CFLAGS=-Wall -Werror -Wextra -std=gnu99 -O2 -g $(INCLUDE) $(LIBDIRS) -D_FILE_OFFSET_BITS=64
POSIX_CFLAGS=$(COMMON_CFLAGS) -pedantic
CFLAGS=$(POSIX_CFLAGS)
WINDOWS_CFLAGS=$(COMMON_CFLAGS) -DWINVER=0x500
WINDOWS_LIBS=-lws2_32 -liberty
APPNAME=audioextract
BIN=$(BUILDDIR)/$(APPNAME)

ifeq ($(TARGET),win32)
	PLATFORM=windows
	CC=i686-pc-mingw32-gcc
	CFLAGS=$(WINDOWS_CFLAGS) -m32
	LDFLAGS=-m32
	LIBS=$(WINDOWS_LIBS)
	APPNAME=audioextract.exe
else
ifeq ($(TARGET),win64)
	PLATFORM=windows
	CC=x86_64-w64-mingw32-gcc
	CFLAGS=$(WINDOWS_CFLAGS) -m64
	LDFLAGS=-m64
	LIBS=$(WINDOWS_LIBS)
	APPNAME=audioextract.exe
else
ifeq ($(TARGET),linux32)
	CFLAGS=$(POSIX_CFLAGS) -m32
	LDFLAGS=-m32
else
ifeq ($(TARGET),linux64)
	CFLAGS=$(POSIX_CFLAGS) -m64
	LDFLAGS=-m64
endif
endif
endif
endif

.PHONY: all clean install uninstall builddir

all: $(BIN)

builddir: $(BUILDDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

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
		src/bink.h \
		src/au.h \
		src/smk.h \
		src/bmp.h \
		src/png.h
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

$(BUILDDIR)/au.o: src/au.c src/audioextract.h src/au.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/smk.o: src/smk.c src/audioextract.h src/smk.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/bmp.o: src/bmp.c src/audioextract.h src/bmp.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/png.o: src/png.c src/audioextract.h src/png.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

ifeq ($(PLATFORM),posix)
install: $(PREFIX)/bin/$(APPNAME)

$(PREFIX)/bin/$(APPNAME): $(BIN)
	mkdir -p "$(PREFIX)/bin"
	install -s $(BIN) "$@"

uninstall:
	rm -f "$(PREFIX)/bin/$(APPNAME)"
endif

clean:
	rm -f $(BIN) $(OBJ)
