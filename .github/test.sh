git clone https://github.com/steveicarus/ivtest.git
cd ivtest

if [ "$OS" = 'Windows_NT' ]; then
  $mingw64 perl update_msys2_report.pl
  $mingw64 perl vvp_reg.pl
  diff --strip-trailing-cr regression_report-msys2.txt regression_report.txt
  $mingw64 perl vpi_reg.pl
else
  perl vvp_reg.pl
  diff regression_report-devel.txt regression_report.txt
  perl vpi_reg.pl
fi
