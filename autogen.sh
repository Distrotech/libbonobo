#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="libbonobo"

(test -f $srcdir/configure.in \
  && test -f $srcdir/autogen.sh \
  && test -d $srcdir/bonobo) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.9 . gnome-autogen.sh
