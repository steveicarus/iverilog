#!/usr/bin/env sh

echo "Using the bundled ivtest to run regression tests."
echo " pwd = $(pwd)"

cd ivtest

status=0

perl vvp_reg.pl || status=1

if [ "x$1" = "xno-pli1" ] ; then
    perl vpi_reg.pl || status=1
else
    perl vpi_reg.pl --with-pli1 || status=1
fi

python3 vvp_reg.py || status=1

exit $status
