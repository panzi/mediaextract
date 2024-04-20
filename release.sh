#!/bin/bash

set -e

pkg=mediaextract-$(git describe --tags)

rm -r "$pkg" "$pkg.zip" 2>/dev/null || echo
git clone . "$pkg/source"
rm -rf "$pkg/source/.git" "$pkg/source/.gitignore"
mv "$pkg/source/README.md" "$pkg"
for target in linux32 linux64 win32 win64; do
	builddir=build-$target
	make TARGET=$target "-j$(nproc)"
	if [[ -d "$builddir" ]]; then
		mkdir "$pkg/$builddir"

		suffix=
		case "$target" in win*) suffix=.exe;; esac

		cp "$builddir/mediaextract$suffix" "$pkg/$builddir"
	fi
done
zip -r9 "$pkg.zip" "$pkg"
