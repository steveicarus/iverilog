#!/bin/sh

#
# This shell script exists to run autoconf on source distributions
# that are pulled from CVS. The configure scripts are not included
# in CVS, and there are several configure.in files, so it is easiest
# to just run this script to autoconf wherever needed.
#
echo "Autoconf in root..."
autoconf

for dir in vpip vvp tgt-vvp
do
    echo "Autoconf in $dir..."
    ( cd $dir ; autoconf )
done
