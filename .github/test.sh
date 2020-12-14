#!/usr/bin/env sh

git clone https://github.com/steveicarus/ivtest.git || exit 1
cd ivtest

version=devel

status=0
if [ "$OS" = 'Windows_NT' ]; then
  perl update_msys2_report.pl $version
  perl vvp_reg.pl || status=1
  diff --strip-trailing-cr regression_report-msys2-$version.txt regression_report.txt || status=1
else
  perl vvp_reg.pl || status=1
  diff regression_report-$version.txt regression_report.txt || status=1
fi

perl vpi_reg.pl || status=1

exit $status
