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

outputRequested=0
target=${vvmTarget}
xnfForm=0

# If VPI module path aren't set up, warn at least
if test -z "${VPI_MODULE_PATH}" ; then
    echo "Missing environment variable VPI_MODULE_PATH.";
    echo "To be able to execute, set VPI_MODULE_PATH to ${VPIModulePath}";
fi

if test -z "$*" ; then
    echo "Missing infile";
    echo "verilog [-Dmacro[=defn]] [-Iincludepath] [-X] [-o outputfilename] sourcefile" ;
    exit -1;
fi

# Try to extract given parameters
for parameter in $*; do 

    if test ${outputRequested} -eq 1 ; then
	outputFile=${parameter};
	outputRequested=0;
    else
	case "${parameter}" in
	    -D*) extDefines="${extDefines} ${parameter}" ;;
	    -I*) extIncPath="${extIncPath} -I ${parameter:2}" ;;
	    -X)  xnfForm=1; target=${xnfTarget} ;;
	    -o)  outputRequested=1 ;;
	    *)   verilogFile=${parameter};;
	esac
    fi

done


# If no output file is given should we guess one or...?
if test -z "${outputFile}" ; then 
    outputFile=`echo ${verilogFile} | sed -e 's/\(.*\)\..*/\1/'`;
    if test ${xnfForm} -eq 1 ; then
	outputFile="${outputFile}.xnf" ;
    fi
fi


# Preprocess
${execIVLPP} ${extDefines} ${extIncPath} -L -o ${tmpPPFile} ${verilogFile}
if test $? -ne 0 ; then
    echo "Preprocessing failed. Terminating compilation."
    rm -f ${tmpPPFile}
    exit -1
fi


# Compile preprocessed verilog file
${execIVL} ${target} -o ${tmpCCFile} ${tmpPPFile}
if test $? -ne  0 ; then
    echo "Verilog compilation failed. Terminating compilation."
    rm -f ${tmpCCFile}
    exit -1
fi
rm -f ${tmpPPFile}

# If XNF just move the created file in place else ...
if test ${xnfForm} -eq 1 ; then
    mv ${tmpCCFile} ${outputFile} ;
else
    # ...compile generated C++ code
    ${execCpp} -rdynamic ${tmpCCFile} -o ${outputFile} -lvvm -ldl
    if test $? -ne 0 ; then
	echo "C++ compilation failed. Terminating compilation."
	rm -f ${tmpCCFile}
	exit -1
    fi
    rm -f ${tmpCCFile}
fi
