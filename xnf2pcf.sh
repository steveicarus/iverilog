#!/bin/sh

# xnf2pcf

# Converts perfectly good EXT records from an XNF file to
# a .pcf file for the "par" step of the Xilinx toolchain.
# Why on earth is this needed?  Oh, well, the joys of working
# with black-box-ware.

# Usage:  xnf2pcf <design.xnf >design.pcf

# Refer to the resulting .pcf file in the invocation of "par", syntax:
#       par [options] infile[.ncd] outfile pcf_file[.pcf]

# Tested (successfully!) with XNF from Icarus Verilog, see
#   http://www.geda.seul.org/tools/verilog/index.html
# and Xilinx back end tools from Foundation 1.5

# Author: Larry Doolittle  <LRDoolittle@lbl.gov>
# Date:   August 19, 1999

echo "SCHEMATIC START ;"
echo "SCHEMATIC END ;"
echo

awk '/^EXT/{gsub(",",""); printf("COMP \"%s\" LOCATE = SITE \"P%s\" ;\n", $2, $4)}'
