Audioextract
============

Extract audio files that are embedded within other files.

Setup
-----

	make
	make install PREFIX=/usr

Usage
-----

	audioextract [option...] <filename> [<filename> ...]

### Examples

Extract .wav, .aif and .ogg (might actually be .flac, .opus or .ogm) files from
the file `data.bin` and store them in the `~/Music` directory.

	audioextract -f wave,ogg -o ~/Music data.bin

Extract .mp3, .mp2 and .mp1 files (with or without ID3v2 tags). The `mpeg` option
yields a lot of false positives because there is no nice way to unambigiously
detect MPEG files. These false positives are however usually very small, so
using the `--min-size` one can hopefully extract only real MPEG files.

	audioextract -f id3v2,mpeg --min-size 100k -o ~/Music data.bin

### Options

	-h, --help             Print this help message.
	-q, --quiet            Do not print status messages.
	-f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.
	                       Supported formats:
	                         all      all supported formats
	                         default  the default set of formats (AIFF, ID3v2, Ogg, RIFF, MIDI, MOD)
	                         aiff     big-endian (Apple) wave files
	                         id3v2    MPEG files with ID3v2 tags at the start
	                         it       ImpulseTracker files
	                         midi     MIDI files
	                         mod      FastTracker files
	                         mpeg     any MPEG files (e.g. MP3)
	                         ogg      Ogg files (Vorbis, FLAC, Opus, Theora, etc.)
	                         riff     little-endian (Windows) wave files
	                         s3m      ScreamTracker files
	                         tracker  all tracker files (MOD, S3M, IT)
	                         wave     both RIFF and AIFF wave files

	                       WARNING: Because MPEG files do not have a nice file magic, using
	                       the 'mpeg' format may cause *a lot* of false positives. Nowadays
	                       MP3 files usually have an ID3v2 tag at the start, so using the
	                       'id3v2' format is the better option anyway.

	                       NOTE: When using only the 'mpeg' format but not 'id3v2' any ID3v2
	                       tag will be stripped. ID3v1 tags will still be kept.

	                       If '-' is written before a format name the format will be
	                       removed from the set of formats to extract. E.g. extract
	                       everything except wave files:

	                         audioextract --formats=all,-wave data.bin

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
