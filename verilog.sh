#!/bin/sh

# verilog - A wrapper shell script for ivl
# Copyright (C) 1999 Stefan Petersen (spe@geda.seul.org)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Added support for the -t flag, for odd-ball target types. <steve@icarus.com>
# Added support for functors, especially for the XNF target. <steve@icarus.com>

# Setup variables
execPath=@bindir@
execIVLPP=${execPath}/ivlpp
execIVL=${execPath}/ivl
execCpp=@CXX@

tmpDir=/tmp
tmpPPFile=${tmpDir}/ivl$$.pp
tmpCCFile=${tmpDir}/ivl$$.cc

includedir=@includedir@
libdir=@libdir@
VPIModulePath=@libdir@/ivl

target="vvm"
targetSuffix=""
functors=""
flags=""

# If VPI module path aren't set up, warn at least
if test -z "${VPI_MODULE_PATH}" ; then
    echo "Missing environment variable VPI_MODULE_PATH.";
    echo "To be able to execute, set VPI_MODULE_PATH to ${VPIModulePath}";
fi

# Try to extract given parameters
parameter=`getopt D:I:Xxf:o:s:t: "$@"` 
eval set -- "${parameter}" 
while true ; do 

    case "$1" in
      -D) extDefines="${extDefines} -D$2" ; shift 2 ;;
      -I) extIncPath="${extIncPath} -I $2" ; shift 2 ;;
      -X) target="xnf" ; shift ;;
      -f) flags="$flags -f$2" ; shift 2 ;;
      -o) outputFile=$2 ; shift 2 ;;
      -s) topModule="-s $2 " ; shift 2 ;;
      -t) target="$2" ; shift 2 ;;
      -x) execute="true"; shift ;;
      --) shift ; break ;;
       *) echo "Internal error! Arg is $1 " ; exit 1 ;;
    esac

done

# The rest is filenames
verilogFile=$@;

if test -z "${verilogFile}" ; then
    echo "Missing infile";
    echo "verilog [-Dmacro[=defn]] [-Iincludepath] [-X] [-x] [-o outputfilename] [-s topmodule] sourcefile[s]" ;
    exit 1;
fi

# Choose a target file suffix based on the target type.
case "$target" in
 vvm) targetSuffix="" ;;
 xnf) targetSuffix=".xnf"
      functors="-Fsynth -Fnodangle -Fxnfio" ;;
   *) targetSuffix=".$target" ;;
esac


# If no output file is given should we guess one or...?
# Assumes a few silly things if several files are given
if test -z "${outputFile}" ; then 
    outputFile=`echo ${verilogFile} | sed -e 's;.* ;;' | sed -e 's;\..*$;;'`
    outputFile="${outputFile}${targetSuffix}" ;
fi

# Preprocess
${execIVLPP} ${extDefines} ${extIncPath} -L -o ${tmpPPFile} ${verilogFile}
if test $? -ne 0 ; then
    echo "Preprocessing failed. Terminating compilation."
    rm -f ${tmpPPFile}
    exit 1
fi


# Compile preprocessed verilog file
${execIVL} -t ${target} -o ${tmpCCFile} ${topModule} ${functors} ${flags} ${tmpPPFile}
if test $? -ne  0 ; then
    echo "Verilog compilation failed. Terminating compilation."
    rm -f ${tmpCCFile}
    exit 1
fi
rm -f ${tmpPPFile}


case "${target}" in

    "xnf") mv ${tmpCCFile} ${outputFile} ;;

    "vvm")  ${execCpp} -rdynamic -I${includedir} -L${libdir} ${tmpCCFile} -o ${outputFile} -lvvm -ldl ;
	    if test $? -ne 0 ; then
	     echo "C++ compilation failed. Terminating compilation."
	     rm -f ${tmpCCFile}
	     exit 1
	    fi
	    rm -f ${tmpCCFile} ;;

    *) mv ${tmpCCFile} ${outputFile} ;;

esac

if test ${execute} ; then
  ./${outputFile}
fi
