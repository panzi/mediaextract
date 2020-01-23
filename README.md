Mediaextract
============

Extract media files that are embedded within other files.

Setup
-----

	make builddir
	make
	sudo make install PREFIX=/usr

Cross compile for Windows (uses `i686-w64-mingw32-gcc`):

	make TARGET=win32 builddir
	make TARGET=win32

Or (uses `x86_64-w64-mingw32-gcc`):

	make TARGET=win64 builddir
	make TARGET=win64

**Warning:** This program only works correctly on platforms that allow unaligned
memory access (e.g. x86 and ARM, although it might be quite slow on the latter).

**Note:** 32bit binaries can only process 2 GB of a file at once. The rest of
bigger files will be ignored. You need to run this program several times with
different `--offset` values to process such a file whole.

This also means that using a 32bit binary extracted files can never be larger
than 2 GB.

This is because `mediaextract` uses `mmap` to read files, which maps files to
memory. On 32bit platforms the address space of the main memory is simply not
big enough. 64bit binaries can read up to 8 EB (8 Exabytes) at once.

Usage
-----

	mediaextract [option...] <filename> [<filename> ...]

### Examples

Extract .wav, .aif and .ogg (might actually be .ogg, .opus or .ogm) files from
the file `data.bin` and store them in the `~/Music` directory.

	mediaextract -f riff,aiff,ogg -o ~/Music data.bin

This will then write files like such into `~/Music`:

	data.bin_00000000.ogg
	data.bin_00FFB2E3.wav
	data.bin_01F3CD45.aif

The hexadecimal number in the written file names give the offset where in the
data file the audio file was found.

Extract .mp3, .mp2 and .mp1 files (with or without ID3v2 tags). The `mpg123`
option yields a lot of false positives because there is no nice way to
unambigiously detect MPEG files. These false positives are however usually very
small, so using the `--min-size` option one can hopefully extract only real MPEG
files.

	mediaextract -f id3v2,mpg123 --min-size=100k -o ~/Music data.bin

### Options

	-h, --help             Print this help message.
	-q, --quiet            Do not print status messages.
	-s, --simulate         Don't write any output files.
	-o, --output=DIR       Directory where extracted files should be written. (default: ".")
	-a, --filename=FORMAT  Format string for the file names.
	                       (default: "{filename}_{offset}.{ext}")

	                       Supported variables:
	                         filename  Filename of the extracted archive.
	                         offset    Offset within the archive in hexadecimal.
	                         index     0-based index of the extracted file in decimal.
	                         size      Size of the extracted file in decimal.
	                         ext       Extension associated with the filetype of the
	                                   extracted file.

	-i, --offset=OFFSET    Start processing at byte OFFSET. (default: 0)
	-n, --length=LENGTH    Only process LENGTH bytes.
	                       (default and maximum: 8 EB)
	-m, --min-size=SIZE    Minumum size of extracted files (skip smaller). (default: 0)
	-x, --max-size=SIZE    Maximum size of extracted files (skip larger).
	                       (default and maximum: 16 EB)

	                       The last character of OFFSET, LENGTH and SIZE may be one of the
	                       following:
	                         B (or none)  for Bytes
	                         k            for Kilobytes (units of 1024 Bytes)
	                         M            for Megabytes (units of 1024 Kilobytes)
	                         G            for Gigabytes (units of 1024 Megabytes)
	                         T            for Terabytes (units of 1024 Gigabytes)
	                         P            for Petabytes (units of 1024 Terabytes)
	                         E            for Exabytes  (units of 1024 Petabytes)

	                       The special value "max" selects the maximum alowed value.

	-f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.

	                       Supported formats:
	                         all      all supported formats
	                         default  the default set of formats (AIFF, ASF, AU, BINK, BMP,
	                                  GIF, ID3v2, IT, JPEG, MPEG 1, MPEG PS, MIDI, MP4, Ogg,
	                                  PNG, RIFF, S3M, SMK, XM, XMIDI)
	                         audio    all audio files (AIFF, ASF, AU, ID3v2, IT, MIDI, MP4,
	                                  Ogg, RIFF, S3M, XM, XMIDI)
	                         text     all text files (ASCII, UTF-8, UTF-16LE, UTF-16BE,
	                                  UTF-32LE, UTF-32BE)
	                         image    all image files (BMP, PNG, JPEG, GIF)
	                         mpeg     all safe mpeg files (MPEG 1, MPEG PS, ID3v2)
	                         tracker  all tracker files (MOD, S3M, IT, XM)
	                         video    all video files (ASF, BINK, MP4, RIFF, SMK)

	                         aiff     big-endian (Apple) wave files
	                         ascii    7-bit ASCII files (only printable characters)
	                         asf      Advanced Systems Format files (also WMA and WMV)
	                         au       Sun Microsystems audio file format (.au or .snd)
	                         bink     BINK files
	                         bmp      Windows Bitmap files
	                         gif      Graphics Interchange Format files
	                         id3v2    MPEG layer 1/2/3 files with ID3v2 tags
	                         it       ImpulseTracker files
	                         jpeg     JPEG Interchange Format files
	                         midi     MIDI files
	                         mod      Noisetracker/Soundtracker/Protracker Module files
	                         mpg123   MPEG layer 1/2/3 files (MP1, MP2, MP3)
	                         mpeg1    MPEG 1 System Streams
	                         mpegps   MPEG 2 Program Streams
	                         mpegts   MPEG 2 Transport Streams
	                         mp4      MP4 files (M4A, M4V, 3GPP etc.)
	                         ogg      Ogg files (Vorbis, Opus, Theora, etc.)
	                         png      Portable Network Graphics files
	                         riff     Resource Interchange File Format files (ANI, AVI, MMM,
	                                  PAL, RDI, RMI, SGT, STY, WAV, WEBP and more)
	                         s3m      ScreamTracker III files
	                         smk      Smaker files
	                         utf-8    7-bit ASCII and UTF-8 files (only printable code points)
	                         utf-16be big-endian UTF-16 files (only printable code points)
	                         utf-16le little-endian UTF-16 files (only printable code points)
	                         utf-32be big-endian UTF-32 files (only printable code points)
	                         utf-32le little-endian UTF-32 files (only printable code points)
	                         xm       Extended Module files
	                         xmidi    XMIDI files

	                       WARNING: Because MP1/2/3 files do not have a nice file magic, using
	                       the 'mpg123' format may cause *a lot* of false positives. Nowadays
	                       MP3 files usually have an ID3v2 tag at the start, so using the
	                       'id3v2' format is the better option anyway.

	                       The detection accuracy of MOD files is not much better and of MPEG TS
	                       it is even worse and thus the 'mpg123', 'mpegts' and 'mod' formats
	                       are per default disabled.

	                       NOTE: When using only the 'mpg123' format but not 'id3v2' any ID3v2
	                       tag will be stripped. ID3v1 tags will still be kept.

	                       If '-' is written before a format name the format will be
	                       removed from the set of formats to extract. E.g. extract
	                       everything except tracker files:

	                         mediaextract --formats=all,-tracker data.bin
