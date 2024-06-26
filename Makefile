PREFIX=/usr/local
TARGET=$(shell uname|tr '[A-Z]' '[a-z]')$(shell getconf LONG_BIT)
INCLUDE=
LIBDIRS=
LIBS=
APPNAME=mediaextract
BINEXT=
MANPAGE=$(APPNAME).1.gz
PLATFORM=posix
BUILD_TYPE=debug
BUILDDIR=build/$(TARGET)/$(BUILD_TYPE)
OBJ=\
	$(BUILDDIR)/mediaextract.o \
	$(BUILDDIR)/mediaextract_$(PLATFORM).o \
	$(BUILDDIR)/formatstring.o \
	$(BUILDDIR)/riff.o \
	$(BUILDDIR)/aiff.o \
	$(BUILDDIR)/ogg.o \
	$(BUILDDIR)/mpg123.o \
	$(BUILDDIR)/mp4.o \
	$(BUILDDIR)/id3.o \
	$(BUILDDIR)/midi.o \
	$(BUILDDIR)/xmidi.o \
	$(BUILDDIR)/mod.o \
	$(BUILDDIR)/s3m.o \
	$(BUILDDIR)/xm.o \
	$(BUILDDIR)/it.o \
	$(BUILDDIR)/asf.o \
	$(BUILDDIR)/bink.o \
	$(BUILDDIR)/au.o \
	$(BUILDDIR)/smk.o \
	$(BUILDDIR)/bmp.o \
	$(BUILDDIR)/png.o \
	$(BUILDDIR)/jpg.o \
	$(BUILDDIR)/avif.o \
	$(BUILDDIR)/gif.o \
	$(BUILDDIR)/mpeg.o \
	$(BUILDDIR)/text.o
CC=gcc
LD=$(CC)
CFLAGS+= -Wall -Werror -Wextra -std=gnu99 $(INCLUDE) $(LIBDIRS) -D_FILE_OFFSET_BITS=64
WINDOWS_LIBS=-lws2_32
BIN=$(BUILDDIR)/$(APPNAME)$(BINEXT)

ifeq ($(BUILD_TYPE),release)
	CFLAGS+=-O2
else
	CFLAGS+=-g
endif

ifeq ($(TARGET),win32)
	PLATFORM=windows
	CC=i686-w64-mingw32-gcc
	CFLAGS+=-DWINVER=0x500 -m32
	LDFLAGS+=-m32
	LIBS=$(WINDOWS_LIBS)
	BINEXT=.exe
else
ifeq ($(TARGET),win64)
	PLATFORM=windows
	CC=x86_64-w64-mingw32-gcc
	CFLAGS+=-DWINVER=0x500 -m64
	LDFLAGS+=-m64
	LIBS=$(WINDOWS_LIBS)
	BINEXT=.exe
else
ifeq ($(TARGET),linux32)
	CFLAGS+=-pedantic -m32
	LDFLAGS+=-m32
else
ifeq ($(TARGET),linux64)
	CFLAGS+=-pedantic -m64
	LDFLAGS+=-m64
endif
endif
endif
endif

.PHONY: all clean install uninstall builddir

ifeq ($(PLATFORM),posix)
all: $(BIN) $(BUILDDIR)/$(MANPAGE)
else
all: $(BIN)
endif

builddir: $(BUILDDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BIN): $(OBJ)
	$(LD) $(LIBDIRS) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

$(BUILDDIR)/mediaextract.o: src/mediaextract.c \
		src/mediaextract.h \
		src/formatstring.h \
		src/ogg.h \
		src/riff.h \
		src/aiff.h \
		src/mpg123.h \
		src/mp4.h \
		src/id3.h \
		src/midi.h \
		src/xmidi.h \
		src/mod.h \
		src/s3m.h \
		src/xm.h \
		src/it.h \
		src/asf.h \
		src/bink.h \
		src/au.h \
		src/smk.h \
		src/bmp.h \
		src/png.h \
		src/jpg.h \
		src/avif.h \
		src/gif.h \
		src/mpeg.h \
		src/text.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mediaextract_$(PLATFORM).o: src/mediaextract_$(PLATFORM).c src/mediaextract.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/formatstring.o: src/formatstring.c src/formatstring.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/%.o: src/%.c src/mediaextract.h src/riff.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

ifeq ($(PLATFORM),posix)
install: $(PREFIX)/bin/$(APPNAME) $(PREFIX)/share/man/man1/$(MANPAGE)

$(BUILDDIR)/$(MANPAGE): src/ManPageIncludeFile $(BIN)
	PATH=$(BUILDDIR):$(PATH) help2man $(APPNAME) --no-info -n "extracts media files that are embedded within other files" -S "Mathias Panzenböck" -i src/ManPageIncludeFile >$(BUILDDIR)/$(APPNAME).1
	gzip -kf $(BUILDDIR)/$(APPNAME).1

$(PREFIX)/share/man/man1/$(MANPAGE): $(BUILDDIR)/$(APPNAME).1
	mkdir -p "$(PREFIX)/share/man/man1/"
	install $(BUILDDIR)/$(MANPAGE) "$@"

$(PREFIX)/bin/$(APPNAME): $(BIN)
	mkdir -p "$(PREFIX)/bin"
	install -s $(BIN) "$@"

uninstall:
	rm -f "$(PREFIX)/bin/$(APPNAME)"
endif

recode: $(BUILDDIR)/recode

$(BUILDDIR)/recode: src/text.c src/text.h
	$(CC) $(CFLAGS) $< -o $@ -DMEDIAEXTRACT_RECODE_BIN

clean:
	rm -f $(BUILDDIR)/*
