#!/bin/sh

#
# This shell script exists to run autoconf on source distributions
# that are pulled from CVS. The configure scripts are not included
# in CVS, and there are several configure.in files, so it is easiest
# to just run this script to autoconf wherever needed.
#
echo "Autoconf in root..."
autoconf -f

echo "Precompiling lexor_keyword.gperf"
gperf -o -i 7 -C -k 1-4,\$ -L ANSI-C -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf > lexor_keyword.cc
