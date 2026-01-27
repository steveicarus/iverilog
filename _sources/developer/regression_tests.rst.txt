
The Regression Test Suite
=========================

Icarus Verilog development includes a regression test suite that is included
along with the source. The "ivtest" directory contains the regression test
suite, and this suite is used by the github actions as continuous integration
to make sure the code is always going forward.

NOTE: There are scripts written in perl to run the regression tests, but they
are being gradually replaced with a newer set of scripts. It is the newer
method that is described here.

Test Descriptions
-----------------

Regression tests are listed in the regress-vvp.list file. Each line lists the
name of the test and the path to the dest description. The list file is
therefore pretty simple, and all the description of the test is in the
description file:

.. code-block:: console

  macro_str_esc  vvp_tests/macro_str_esc.json

The "name" is a simple name, and the test-description-file is the path (relative
the ivtest directory) to the description file. A simple test description file
is a JSON file, like this:

.. code-block:: java

  {
    "type"   : "normal",
    "source" : "macro_str_esc.v",
    "gold"   : "macro_str_esc"
  }

This description file contains all the information that the vvp_reg.py script
needs to run the regression test. The sections below describe the keys and
values in the description file dictionary.

source (required)
^^^^^^^^^^^^^^^^^
This specifies the name of the source file. The file is actually to be found
in the ivltests/ directory.


type (required)
^^^^^^^^^^^^^^^

This describes the kind of test to run. The valid values are:

* **normal** - Compile the source using the iverilog compiler vvp target, and if
  that succeeds execute it using the vvp command. If there is no gold file
  specified, then look for an output line with the "PASSED" string.

* **normal-vlog95** - This is similar to the normal case, but uses
  the -tvlog95 target in a first pass to generate simplified verilog, then a
  regular iverilog command with the -tvvp target to generate the actual
  executable. This tests the -tvlog95 target.

* **NI** - Mark the test as not implemented. The test will be skipped without
  running or reporting an error.

* **CE** - Compile, but expect the compiler to fail. This means the compiler
  command process must return an error exit.

* **EF** - Compile and run, but expect the run time to fail. This means the
  run time program must return an error exit.

gold (optional)
^^^^^^^^^^^^^^^

If this is specified, it replaces the "Passed" condition with a comparison of
the output with a gold file. The argument is the name of the gold file set,
which will be found in the "gold/" directory. The name here is actually the
basename of the gold files, with separate actual gold files for the iverilog
and vvp stderr and stdout. For example, if a "normal" test includes a gold
file, then the program is compiled and run, and the outputs are compared with
the gold file to make sure it ran properly.

The way the regression suite works, there are 4 log files created for each
test:

* foo-iverilog-stdout.log
* foo-iverilog-stderr.log
* foo-vvp-stdout.log
* foo-vvp-stderr.log

The "gold" value is the name of the gold file set. If the gold value is "foo",
Then the actual gold files are called:

* gold/foo-iverilog-stdout.gold
* gold/foo-iverilog-stderr.gold
* gold/foo-vvp-stdout.gold
* gold/foo/vvp-stderr.gold

If any of those files is empty, then the gold file doesn't need to be
present at all. The log files and the gold files are compared byte for
byte, so if the output you are getting is correct, then copy the log to
the corresponding gold, and you're done.

If the run type is "CE" or "RE", then the gold files still work, and can
be used to check that the error message is correct. If the gold file setting
is present, the error return is required, and also the gold files must match.

iverilog-args (optional)
^^^^^^^^^^^^^^^^^^^^^^^^

If this is specified, it is a list of strings that are passed as arguments to
the iverilog command line.

vvp-args (optional)
^^^^^^^^^^^^^^^^^^^^

If this is specified, it is a list of strings that are passed as arguments to
the vvp command. These arguments go before the vvp input file that is to be
run.

vvp-args-extended (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If this is specified, it is a lost of strings that are passed as arguments to
the vvp command. These are extended arguments, and are placed after the vvp
input file that is being run. This is where you place things like plusargs.
