Audioextract
============

Extract audio files that are embedded within other files.

Setup
-----

	make
	make install PREFIX=/usr

Cross compile for Windows:

	make TARGET=win32

Or:

	make TARGET=win64

Note that 32bit binaries can only read files up to about 2 GB because of the
limitations of the 32bit address space. 64bit binaries can read up to about
8 EB (8 Exabyte).

Usage
-----

	audioextract [option...] <filename> [<filename> ...]

### Examples

Extract .wav, .aif and .ogg (might actually be .flac, .opus or .ogm) files from
the file `data.bin` and store them in the `~/Music` directory.

	audioextract -f wave,ogg -o ~/Music data.bin

This will then write files like such into `~/Music`:

	data.bin_00000000.ogg
	data.bin_00FFB2E3.wav
	data.bin_01F3CD45.aif

The hexadecimal number in the written file names give the offset where in the
data file the audio file was found.

Extract .mp3, .mp2 and .mp1 files (with or without ID3v2 tags). The `mpg123`
option yields a lot of false positives because there is no nice way to
unambigiously detect MPEG files. These false positives are however usually very
small, so using the `--min-size` one can hopefully extract only real MPEG files.

	audioextract -f id3v2,mpg123 --min-size 100k -o ~/Music data.bin

### Options

	-h, --help             Print this help message.
	-q, --quiet            Do not print status messages.
	-f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.

	                       Supported formats:
	                         all      all supported formats
	                         default  the default set of formats (AIFF, ASF, BINK, ID3v2, IT,
	                                  MIDI, MP4, Ogg, RIFF, S3M, XM)
	                         aiff     big-endian (Apple) wave files
	                         asf      Advanced Systems Format files (also WMA and WMV)
	                         bink     BINK files
	                         id3v2    MPEG layer 1/2/3 files with ID3v2 tags
	                         it       ImpulseTracker files
	                         midi     MIDI files
	                         mod      Noisetracker/Soundtracker/Protracker Module files
	                         mpg123   MPEG layer 1/2/3 files (MP1, MP2, MP3)
	                         mp4      MP4 files (M4A, M4V, 3GPP etc.)
	                         ogg      Ogg files (Vorbis, Opus, Theora, etc.)
	                         riff     Resource Interchange File Format files (ANI, AVI, MMM,
	                                  PAL, RDI, RMI, WAV)
	                         s3m      ScreamTracker III files
	                         xm       Extended Module files
	                         tracker  all tracker files (MOD, S3M, IT, XM)

	                       WARNING: Because MP1/2/3 files do not have a nice file magic, using
	                       the 'mpg123' format may cause *a lot* of false positives. Nowadays
	                       MP3 files usually have an ID3v2 tag at the start, so using the
	                       'id3v2' format is the better option anyway.

	                       The detection accuracy of MOD files is not much better and thus
	                       they are also per default disabled.

	                       NOTE: When using only the 'mpg123' format but not 'id3v2' any ID3v2
	                       tag will be stripped. ID3v1 tags will still be kept.

	                       If '-' is written before a format name the format will be
	                       removed from the set of formats to extract. E.g. extract
	                       everything except tracker files:

	                         audioextract --formats=all,-tracker data.bin

	-o, --output=DIR       Directory where extracted files should be written. (default: ".")
	-m, --min-size=SIZE    Minumum size of extracted files (skip smaller). (default: 0)
	-x, --max-size=SIZE    Maximum size of extracted files (skip larger).
	                       (default: max. possible size_t value)

	                       The last character of SIZE may be one of the following:
	                         B (or none)  for bytes
	                         k            for Kilobytes (units of 1024 bytes)
	                         M            for Megabytes (units of 1024 Kilobytes)
	                         G            for Gigabytes (units of 1024 Megabytes)
	                         T            for Terabytes (units of 1024 Gigabytes)
