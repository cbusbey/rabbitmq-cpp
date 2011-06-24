#!/bin/bash

rm -rf *.cache
if [ -d "configure.d" ]; then
	rm -rf configure.d/
fi
mkdir configure.d

LIBTOOLIZE=libtoolize
which glibtoolize >/dev/null 2>&1
if [ $? = 0 ]; then
	LIBTOOLIZE=glibtoolize
fi

#----

aclocal
autoheader
$LIBTOOLIZE --force --copy --automake
automake --foreign --add-missing --copy
autoconf

#----
