#!/bin/sh
#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    Library General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this program; if not, write to the Free
#    Software Foundation, Inc.,
#    59 Temple Place - Suite 330
#    Boston, MA 02111-1307, USA
#
#ident "$Id: iverilog-vpi.sh,v 1.3 2002/05/15 01:25:25 steve Exp $"

# These are the variables used for compiling files
CC=gcc
CXX=gcc
CFLAGS="@PIC@ -O"

# These are used for linking...
LD=gcc
LDFLAGS="@SHARED@"
LDLIBS=-lvpi

CCSRC=
CXSRC=
OBJ=
LIB=
OUT=

# --
# parse the command line switches. This collects the source files
# and precompiled object files, and maybe user libraries. As we are
# going, guess an output file name.
for parm
do
    case $parm
    in

    *.c) CCSRC="$SRC $parm"
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .c`
	 fi
	 ;;

    *.cc) CXSRC="$SRC $parm"
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .cc`
	 fi
	 ;;

    *.o) OBJ="$OBJ $parm"
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .o`
	 fi
	 ;;

    -l*) LIB="$LIB $parm"
	 ;;

    esac

done

if [ x$OUT = x ]; then
    echo "Usage: vpi-tool [src and obj files]..."
    exit 0
fi

# Put the .vpi on the result file.
OUT=$OUT".vpi"

compile_errors=0

# Compile all the source files into object files
for src
in $CCSRC
do
    base=`basename $src .c`
    obj=$base".o"

    echo "Compiling $src..."
    $CC -c -o $obj $src || compile_errors=`expr $compile_errors + 1`
    OBJ="$OBJ $obj"
done

for src
in $CXSRC
do
    base=`basename $src .cc`
    obj=$base".o"

    echo "Compiling $src..."
    $CXX -c -o $obj $src || compile_errors=`expr $compile_errors + 1`
    OBJ="$OBJ $obj"
done

if test $compile_errors -gt 0
then
    echo "Some ($compile_errors) files failed to compile."
    exit $compile_errors
fi

echo "Making $OUT from $OBJ..."
exec $LD -o $OUT $LDFLAGS $OBJ $LDLIBS
