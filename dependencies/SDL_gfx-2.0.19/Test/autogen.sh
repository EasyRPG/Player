#!/bin/sh
#
aclocal
automake --foreign
autoconf

#./configure $*
echo "Now you are ready to run ./configure"
