#!/usr/bin/env sh

echo "Using the bundled ivtest to run regression tests."
echo " pwd = $(pwd)"

cd ivtest

status=0

perl vvp_reg.pl || status=1

perl vpi_reg.pl || status=1

python3 vvp_reg.py || status=1

exit $status
