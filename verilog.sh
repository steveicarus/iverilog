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


# Setup variables
execPath=@bindir@
execIVLPP=${execPath}/ivlpp
execIVL=${execPath}/ivl
execCpp=/usr/bin/g++

vvmTarget="-t vvm"
xnfTarget="-t xnf"

tmpDir=/tmp
tmpPPFile=${tmpDir}/ivl$$.pp
tmpCCFile=${tmpDir}/ivl$$.cc

VPIModulePath=@libdir@/ivl:.

target=${vvmTarget}
targetSuffix=""

# If VPI module path aren't set up, warn at least
if test -z "${VPI_MODULE_PATH}" ; then
    echo "Missing environment variable VPI_MODULE_PATH.";
    echo "To be able to execute, set VPI_MODULE_PATH to ${VPIModulePath}";
fi

if test -z "$*" ; then
    echo "Missing infile";
    echo "verilog [-Dmacro[=defn]] [-Iincludepath] [-X] [-o outputfilename] [-s topmodule] sourcefile[s]" ;
    exit -1;
fi

# Try to extract given parameters
parameter=`getopt -o D:I:Xo:s: -- "$@"` 
eval set -- "${parameter}" 
while true ; do 

    case "$1" in
      -D) extDefines="${extDefines} -D$2" ; shift 2 ;;
      -I) extIncPath="${extIncPath} -I $2" ; shift 2 ;;
      -X) targetSuffix=".xnf" ; target=${xnfTarget} ; shift ;;
      -o) outputFile=$2 ; shift 2 ;;
      -s) topModule="-s $2 " ; shift 2 ;;
      --) shift ; break ;;
       *) echo "Internal error! Arg is $1 " ; exit 1 ;;
    esac

done

# The rest is filenames
verilogFile=$@;

# If no output file is given should we guess one or...?
# Assumes a few silly things if several files are given
if test -z "${outputFile}" ; then 
    outputFile=`echo ${verilogFile} | sed -e 's;\(.* \+\)*\(.*\)\..*;\2;'`;
    outputFile="${outputFile}${targetSuffix}" ;
fi


# Preprocess
${execIVLPP} ${extDefines} ${extIncPath} -L -o ${tmpPPFile} ${verilogFile}
if test $? -ne 0 ; then
    echo "Preprocessing failed. Terminating compilation."
    rm -f ${tmpPPFile}
    exit -1
fi


# Compile preprocessed verilog file
${execIVL} ${target} -o ${tmpCCFile} ${topModule} ${tmpPPFile}
if test $? -ne  0 ; then
    echo "Verilog compilation failed. Terminating compilation."
    rm -f ${tmpCCFile}
    exit -1
fi
rm -f ${tmpPPFile}


case "${targetSuffix}" in

    .xnf) mv ${tmpCCFile} ${outputFile} ;;

    "")  ${execCpp} -rdynamic ${tmpCCFile} -o ${outputFile} -lvvm -ldl ;
         if test $? -ne 0 ; then
	     echo "C++ compilation failed. Terminating compilation."
	     rm -f ${tmpCCFile}
	     exit -1
          fi
          rm -f ${tmpCCFile} ;;

    *) echo "Internal error in target compilation." ; exit 1

esac
