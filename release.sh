#!/bin/bash

set -e

pkg=mediaextract-$(git describe --tags)

rm -r "$pkg" "$pkg.zip" 2>/dev/null || echo
git clone . "$pkg/source"
rm -rf "$pkg/source/.git" "$pkg/source/.gitignore"
mv "$pkg/source/README.md" "$pkg"
for target in linux32 linux64 win32 win64; do
	builddir=build/$target/release
	make TARGET=$target BUILD_TYPE=release builddir
	make TARGET=$target BUILD_TYPE=release "-j$(nproc)"
	if [[ -d "$builddir" ]]; then
		mkdir "$pkg/$target"

		suffix=
		case "$target" in win*) suffix=.exe;; esac

		cp "$builddir/mediaextract$suffix" "$pkg/$target"
	fi
done
zip -r9 "$pkg.zip" "$pkg"
