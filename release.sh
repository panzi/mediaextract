#!/bin/bash

pkg=mediaextract-`git describe --tags`

rm -r $pkg $pkg.zip
git clone . $pkg/source
rm -rf $pkg/source/.git $pkg/source/.gitignore
mv $pkg/source/README.md $pkg
for target in linux32 linux64 win32 win64; do
	builddir=build-$target
	make TARGET=$target || exit $1
	if [ -d $builddir ]; then
		mkdir $pkg/$builddir

		if [ -f $builddir/mediaextract ]; then
			cp $builddir/mediaextract $pkg/$builddir
		elif [ -f $builddir/mediaextract.exe ]; then
			cp $builddir/mediaextract.exe $pkg/$builddir
		fi
	fi
done
zip -r9 $pkg.zip $pkg
