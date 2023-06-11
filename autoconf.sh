#!/bin/sh

#
# This shell script exists to run autoconf on source distributions
# that are pulled from git The configure script is not included
# in git, so it is easiest to just run this script whenever needed
# to generate the configure script.
#
# wget -O config.guess 'https://git.savannah.gnu.org/cgit/config.git/plain/config.guess'
# wget -O config.sub 'https://git.savannah.gnu.org/cgit/config.git/plain/config.sub'
echo "Autoconf in root..."
autoconf -f

echo "Precompiling lexor_keyword.gperf"
gperf -o -i 7 -C -k 1-4,6,9,\$ -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf > lexor_keyword.cc

echo "Precompiling vhdlpp/lexor_keyword.gperf"
(cd vhdlpp ; gperf -o -i 7 --ignore-case -C -k 1-4,6,9,\$ -H keyword_hash -N check_identifier -t ./lexor_keyword.gperf > lexor_keyword.cc )
