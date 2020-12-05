#!/usr/bin/env sh

git clone https://github.com/steveicarus/ivtest.git
cd ivtest

version=devel

if [ "$OS" = 'Windows_NT' ]; then
  perl update_msys2_report.pl $version
  perl vvp_reg.pl
  diff --strip-trailing-cr regression_report-msys2-$version.txt regression_report.txt
else
  perl vvp_reg.pl
  diff regression_report-$version.txt regression_report.txt
fi

perl vpi_reg.pl
