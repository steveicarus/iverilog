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

target="-t vvm"

tmpDir=/tmp
tmpPPFile=${tmpDir}/ivl$$.pp
tmpCCFile=${tmpDir}/ivl$$.cc

VPIModulePath=@libdir@/ivl:.

outputRequested=0


# If VPI module path aren't set up, setup default
# Humm, doesn't work as intended ... silly silly me
if test -z "${VPI_MODULE_PATH}" ; then
  VPI_MODULE_PATH=${VPIModulePath} ;
  export VPI_MODULE_PATH ;
fi


# Try to extract given parameters
for parameter in $*; do 

#  echo ${parameter} ;

  if test ${outputRequested} -eq 1 ; then
    outputFile=${parameter};
    outputRequested=0;
  else
    case "${parameter}" in
      -D*) extDefines="${extDefines} ${parameter}" ;;
      -I*) extIncPath="${extIncPath} -I ${parameter:2}";;
      -l*) extLibPath="${extLibPath} ${parameter}" ;;
      -o)  outputRequested=1 ;;
      *)   verilogFile=${parameter};;
    esac
  fi

done


# If no output file is given should we guess one or...?
if test -z "${outputFile}" ; then 
  outputFile=`echo ${verilogFile} | sed -e 's/\(.*\)\..*/\1/'`;
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

# Compile generated C++ code
${execCpp} -rdynamic ${tmpCCFile} -o ${outputFile} -lvvm -ldl
if test $? -ne 0 ; then
  echo "C++ compilation failed. Terminating compilation."
  rm -f ${tmpCCFile}
  exit -1
fi
rm -f ${tmpCCFile}
