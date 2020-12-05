#!/usr/bin/env sh

suffix=-11

git clone https://github.com/steveicarus/ivtest.git
cd ivtest

if [ "$OS" = 'Windows_NT' ]; then
  perl update_msys2_report.pl
  perl vvp_reg.pl $suffix
  diff --strip-trailing-cr regression_report-msys2.txt regression_report.txt
else
  perl vvp_reg.pl $suffix
  diff regression_report-devel.txt regression_report.txt
fi

perl vpi_reg.pl $suffix
