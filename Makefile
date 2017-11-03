PREFIX=/usr/local
TARGET=$(shell uname|tr '[A-Z]' '[a-z]')$(shell getconf LONG_BIT)
INCLUDE=
LIBDIRS=
LIBS=
MANPAGE=mediaextract.1.gz
PLATFORM=posix
BUILDDIR=build-$(TARGET)
OBJ=\
	$(BUILDDIR)/mediaextract.o \
	$(BUILDDIR)/mediaextract_$(PLATFORM).o \
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
	$(BUILDDIR)/gif.o \
	$(BUILDDIR)/mpeg.o \
	$(BUILDDIR)/text.o
CC=gcc
LD=$(CC)
CFLAGS+= -Wall -Werror -Wextra -std=gnu99 -O2 -g $(INCLUDE) $(LIBDIRS) -D_FILE_OFFSET_BITS=64
WINDOWS_LIBS=-lws2_32
APPNAME=mediaextract
BIN=$(BUILDDIR)/$(APPNAME)

ifeq ($(TARGET),win32)
	PLATFORM=windows
	CC=i686-w64-mingw32-gcc
	CFLAGS+=-DWINVER=0x500 -m32
	LDFLAGS+=-m32
	LIBS=$(WINDOWS_LIBS)
	APPNAME=mediaextract.exe
else
ifeq ($(TARGET),win64)
	PLATFORM=windows
	CC=x86_64-w64-mingw32-gcc
	CFLAGS+=-DWINVER=0x500 -m64
	LDFLAGS+=-m64
	LIBS=$(WINDOWS_LIBS)
	APPNAME=mediaextract.exe
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

all: $(BIN) $(BUILDDIR)/$(MANPAGE)

builddir: $(BUILDDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BIN): $(OBJ)
	$(LD) $(LIBDIRS) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

$(BUILDDIR)/mediaextract.o: src/mediaextract.c \
		src/mediaextract.h \
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
		src/gif.h \
		src/mpeg.h \
		src/text.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mediaextract_$(PLATFORM).o: src/mediaextract_$(PLATFORM).c src/mediaextract.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/riff.o: src/riff.c src/mediaextract.h src/riff.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/aiff.o: src/aiff.c src/mediaextract.h src/aiff.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/ogg.o: src/ogg.c src/mediaextract.h src/ogg.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mpg123.o: src/mpg123.c src/mediaextract.h src/mpg123.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mp4.o: src/mp4.c src/mediaextract.h src/mp4.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/id3.o: src/id3.c src/mediaextract.h src/id3.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/midi.o: src/midi.c src/mediaextract.h src/midi.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/xmidi.o: src/xmidi.c src/mediaextract.h src/xmidi.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mod.o: src/mod.c src/mediaextract.h src/mod.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/s3m.o: src/s3m.c src/mediaextract.h src/s3m.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/xm.o: src/xm.c src/mediaextract.h src/xm.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/it.o: src/it.c src/mediaextract.h src/it.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/asf.o: src/asf.c src/mediaextract.h src/asf.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/bink.o: src/bink.c src/mediaextract.h src/bink.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/au.o: src/au.c src/mediaextract.h src/au.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/smk.o: src/smk.c src/mediaextract.h src/smk.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/bmp.o: src/bmp.c src/mediaextract.h src/bmp.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/png.o: src/png.c src/mediaextract.h src/png.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/jpg.o: src/jpg.c src/mediaextract.h src/jpg.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/gif.o: src/gif.c src/mediaextract.h src/gif.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/mpeg.o: src/mpeg.c src/mediaextract.h src/mpeg.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

$(BUILDDIR)/text.o: src/text.c src/mediaextract.h src/text.h
	$(CC) $(CFLAGS) $< -o $@ -c $(LIBS)

ifeq ($(PLATFORM),posix)
install: $(PREFIX)/bin/$(APPNAME) $(PREFIX)/share/man/man1/$(MANPAGE)

$(BUILDDIR)/$(MANPAGE):src/ManPageIncludeFile
	help2man $(BIN) --no-discard-stderr --no-info -n "extracts media files that are embedded within other files" -S "Mathias Panzenböck" -i src/ManPageIncludeFile|sed '/the default set of formats/s/^/.TP\n/'|sed '/the default set of formats/{N;s/.TP//}'|sed -r '/([)]|files)$ /a .TP' >$(BUILDDIR)/$(APPNAME).1
	gzip -kf $(BUILDDIR)/$(APPNAME).1

$(PREFIX)/share/man/man1/$(MANPAGE):$(BUILDDIR)/$(APPNAME).1
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
	rm -rf $(BUILDDIR)
