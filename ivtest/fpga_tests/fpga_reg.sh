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
#ident "$Id: fpga_reg.sh,v 1.5 2004/01/13 03:37:04 stevewilliams Exp $"

# This script runs the synthesis tests listed in the fpga_reg.list
# list file. The script uses Icarus Verilog from the path, and also
# gets ngdbuild and ngd2ver from the path. The XILINX variable needs
# to point to the XILINX install directory so that the simprims
# can be found. The run test uses these to generate a simulation
# from the synthesized file.
#
# Usage: sh ./fpga_reg.sh [select]
#
#  If there is no select, then run all the tests. If there is a select,
#  then only run the tests that match the select regular expression.
#

# This is a diff command for comparing log with gold files.
diff="diff --strip-trailing-cr -aq"

# This is the output file.
status_file=fpga_reg.txt
true > $status_file

if ! test -d fpga_log
then
    mkdir fpga_log
fi

if ! test -d fpga_tmp
then
    mkdir fpga_tmp
fi

if test "X$1" = "X"; then
    match='.*'
else
    match="$1"
fi

cat fpga_reg.list |
 sed -e 's/#.*//' |
 while read test tb arch part gold junk
 do
    if test "X$test" = "X" -o 0 = `expr X$test : X$match`
    then
	: skip a comment
    else
	if test "X$part" != "X-"
	then
	    part="-ppart=$part"
	else
	    part=
	fi

	true > fpga_log/$test-$arch.log 2>&1
	EDIF="$test-$arch.edf"

	synth="iverilog -ofpga_tmp/$EDIF -tfpga -parch=$arch $part $test.v"
	echo "synth=$synth"
	eval "$synth" > fpga_log/$test-$arch-synth.log 2>&1
	if test $? != 0
	then
	    echo "$test-$arch: FAILED -- Synthesis error" >> $status_file
	    continue
	fi

	ngdbuild="ngdbuild $EDIF $test.ngd"
	echo "ngdbuild=$ngdbuild"
	(eval "cd fpga_tmp; $ngdbuild") > fpga_log/$test-$arch-build.log 2>&1
	if test $? != 0
	then
	    echo "$test-$arch: FAILED -- ngdbuild error" >> $status_file
	    continue
	fi

	ngd2ver="ngd2ver -w $test.ngd $test.edf.v"
	echo "ngd2ver=$ngd2ver"
	(eval "cd fpga_tmp; $ngd2ver") > fpga_log/$test-$arch-ngd2ver.log 2>&1
	if test $? != 0
	then
	    echo "$test-$arch: FAILED -- ngd2ver error" >> $status_file
	    continue
	fi

	iverilog -oa.out -Ttyp $tb.v fpga_tmp/$test.edf.v $XILINX/verilog/src/glbl.v -y $XILINX/verilog/src/simprims
	if test $? != 0
	then
	    echo "$test-$arch: FAILED -- compiling test bench" >> $status_file
	    continue
	fi

	vvp a.out > fpga_log/$test-$arch.log 2>&1
	if test "X$gold" != "X-" ; then
	    if $diff $gold fpga_log/$test-$arch.log > /dev/null
	    then
		echo "$test-$arch: PASSED -- Correct output." >> $status_file
            else
	        echo "$test-$arch: FAILED -- Incorrect output." >> $status_file
	    fi
	else
	    if grep -a -q PASSED fpga_log/$test-$arch.log
	    then
		echo "$test-$arch: PASSED" >> $status_file
	    else
		echo "$test-$arch: FAILED" >> $status_file
	    fi
	fi
	rm a.out
    fi
 done

PASSED=`grep ': PASSED' $status_file | wc -l`
FAILED=`grep ': FAILED' $status_file | wc -l`
echo "PASSED=$PASSED, FAILED=$FAILED" >> $status_file

# $Log: fpga_reg.sh,v $
# Revision 1.5  2004/01/13 03:37:04  stevewilliams
#  Cope with dos line-ends while comparing gold files.
#
# Revision 1.4  2003/04/01 05:58:36  stevewilliams
#  Add a select argument.
#
