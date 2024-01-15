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
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#    Boston, MA 02111-1307, USA
#

# These are the variables used for compiling files
CC="@IVCC@"
CXX="@IVCXX@"
CFLAGS="@PIC@ @IVCFLAGS@ -I@INCLUDEDIR@"
CXXFLAGS="@PIC@ @IVCXXFLAGS@ -I@INCLUDEDIR@"

SUFFIX=@SUFFIX@

# These are used for linking...
LD=$CC
LDFLAGS="@IVCTARGETFLAGS@ @SHARED@ -L@LIBDIR@"
LDLIBS="-lveriuser$SUFFIX -lvpi$SUFFIX"

CCSRC=
CXSRC=
OBJ=
LIB=
LIBDIR=
OUT=
INCOPT=
DEFS=

# --
# parse the command line switches. This collects the source files
# and precompiled object files, and maybe user libraries. As we are
# going, guess an output file name.
for parm
do
    case $parm
    in

    *.c) CCSRC="$CCSRC $parm"
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .c`
	 fi
	 ;;

    *.cc) CXSRC="$CXSRC $parm"
	 LD=$CXX
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .cc`
	 fi
	 ;;

    *.cpp) CXSRC="$CXSRC $parm"
	 LD=$CXX
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .cpp`
	 fi
	 ;;

    *.o) OBJ="$OBJ $parm"
         if [ x$OUT = x ]; then
	    OUT=`basename $parm .o`
	 fi
	 ;;

    --name=*)
	 OUT=`echo $parm | cut -b8-`
	 ;;

    -l*) LIB="$LIB $parm"
	 ;;

    -L*) LIBDIR="$LIBDIR $parm"
	 ;;

    -I*) INCOPT="$INCOPT $parm"
	 ;;

    -D*) DEFS="$DEFS $parm"
	 ;;

    --cflags)
	 echo "$CFLAGS"
	 exit;
	 ;;

    --ccflags)
	 echo "$CXXFLAGS"
	 exit;
	 ;;

    --ldflags)
	 echo "$LDFLAGS"
	 exit;
	 ;;

    --ldlibs)
	 echo "$LDLIBS"
	 exit;
	 ;;

    --install-dir)
	 echo "@LIBDIR@/ivl$SUFFIX"
	 exit
	 ;;
    esac

done

if [ x$OUT = x ]; then
    echo "Usage: $0 [src and obj files]..." 1>&2
    exit 0
fi

# Put the .vpi on the result file.
OUT=$OUT".vpi"

compile_errors=0

# Compile all the source files into object files
for src in $CCSRC
do
    base=`basename $src .c`
    obj=$base".o"

    echo "Compiling $src..."
    $CC -c -o $obj $DEFS $CFLAGS $INCOPT $src || compile_errors=`expr $compile_errors + 1`
    OBJ="$OBJ $obj"
done

for src in $CXSRC
do
    base=`basename $src .cc`
    obj=$base".o"

    echo "Compiling $src..."
    $CXX -c -o $obj $DEFS $CXXFLAGS $INCOPT $src || compile_errors=`expr $compile_errors + 1`
    OBJ="$OBJ $obj"
done

if test $compile_errors -gt 0
then
    echo "$0: $compile_errors file(s) failed to compile."
    exit $compile_errors
fi

echo "Making $OUT from $OBJ..."
exec $LD -o $OUT $LDFLAGS $LIBDIR $OBJ $LIB $LDLIBS
