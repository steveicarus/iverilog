####################
#
# Main test script
#
####################

There are a group of tests that are meant to exercise the compiler
and the run time. To run them just type:

./regress

or

perl vvp_reg.pl

or if perl is located in /usr/bin

./vvp_reg.pl

The output from these tests are displayed on the screen
and are also placed in the regression_report.txt file.
The expected output for the current development release
is located in the regression_report-devel.txt file. The
expected output for stable (released) versions can be
found in files named regression_report-v<version>.txt.

The results from individual tests can be found in the
log directory and gold files, when needed, are in the
gold directory. The source files can be found in the
ivltests and contrib directories. The list of tests
and how they are run are in the regress-*.list files.

To check a specific suffixed version of Icarus Verilog
use the --suffix=<suffix> flag to tell the script which
version to run e.g.(--suffix=-10 will test iverilog-10,
etc.). You can also run the test with valgrind (very very
slow) by giving the script the --with-valgrind flag.


####################
#
# VPI test script
#
####################

To test the VPI interface type:

perl vpi_reg.pl

or if perl is located in /usr/bin

./vpi_reg.pl

All these tests should pass.

The individual test results are found in the vpi_log
directory and the gold files are in the vpi_gold
directory. The source files are in the vpi directory.
The vpi_regress.list file has the tests to perform.

This script also takes the --suffix=<suffix> and the
--with-valgrind flags described above.


####################
#
# VHDL test script
#
####################

** Note this is no longer maintained **

This test script require that ghdl be installed in your
path and is used to test the Verilog to VHDL translation.

perl vhdl_reg.pl

or if perl is located in /usr/bin

./vhdl_reg.pl

The tests are expected to pass except for the tri* tests.

This script also takes the --suffix=<suffix> and the
--with-valgrind flags described above.


####################
#
# BLIF test script
#
####################

This test script require that abc be installed in your
path and is used to test the Verilog to VHDL translation.

python blif_reg.py

There is no expected output as of yet so to check for
regressions simply run with and without your patches.


####################
#
# Windows (MinGW) test issues
#
####################

When running under Windows using a MinGW build in a MSYS2
shell, the expected output from vvp_reg.pl can be found in
regression_report-msys2.txt. The MinGW/MSYS2 specific test
exceptions can be found in regress-msys2.list. Exceptions
for the VPI tests can be found in the vpi_regress.list file.

With Windows 10 and MSYS2, there are now very few differences
between the Windows and Linux builds.
