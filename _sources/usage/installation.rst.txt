
Installation Guide
==================

Icarus Verilog may be installed from source code (either from ``git`` or a
released `tar/zip` file), or from pre-packaged binary distributions. If you
don't have a need for the very latest, and prepackaged binaries are available,
that is the easiest place to start.

Installation From Source
------------------------

Icarus is developed for Unix-like environments but can also be compiled on
Windows systems using the `Cygwin/MSYS2` environments or `MinGW` compilers. The
following instructions are the common steps for obtaining the Icarus Verilog
source code, compiling, installing, and checking the compiled code is working
properly. Note that there are pre-compiled and/or prepackaged versions for a
variety of systems, so if you find an appropriate packaged version, then that
is the easiest way to install.

The source code for Icarus is stored under the `git` source code control
system. You can use ``git`` to get the latest development head or the latest of
a specific branch. Stable releases are placed on branches, and in particular V12
stable releases are on the branch "v12-branch" To get the development version
of the code follow these steps::

  % git config --global user.name "Your Name Goes Here"
  % git config --global user.email you@yourpublicemail.example.com
  % git clone https://github.com/steveicarus/iverilog.git

The first two lines are optional and are used to tell git who you are. This
information is important if/when you submit a patch. We suggest that you add
this information now so you don't forget to do it later. The clone will create
a directory, named `iverilog`, containing the source tree, and will populate
that directory with the most current source from the HEAD of the repository.

Change into this directory using::

  % cd iverilog

Normally, this is enough as you are now pointing at the most current
development code, and you have implicitly created a branch `master` that
tracks the development head. However, If you want to actually be working on
the `v12-branch` (the branch where the latest V12 patches are) then you
checkout that branch with the command::

  % git checkout --track -b v12-branch origin/v12-branch

This creates a local branch that tracks the `v12-branch` in the repository, and
switches you over to your new `v12-branch`. The tracking is important as it
causes pulls from the repository to re-merge your local branch with the remote
`v12-branch`. You always work on a local branch, then merge only when you
push/pull from the remote repository.

The choice between the development branch and the latest released branch
depends on your stability requirements. The released branch will only get bug
fixes. It will not get any enhancements or changes in the compiler output
format. Unlike many project the development branch is fairly stable with only
occasional periods of instability. We do most of our big changes in side
branches and only merge them into the development branch when they are clean.

Now that you've cloned the repository and optionally selected the branch you
want to work on, your local source tree may later be synced up with the
development source by using the git command::

  % git pull

The git system remembers the repository that it was cloned from, so you don't
need to re-enter it when you pull.

To build the `configure` script and hash files you need to run the
following::

  % sh autoconf.sh
  % cd ..

This is not need for the released `tar/zip` files since they already contain
these files. You only need to run this once after cloning. If you are missing
``autoconf`` or ``gperf`` then the script will fail::

  Autoconf in root...
  autoconf.sh: 10: autoconf: not found
  Precompiling lexor_keyword.gperf
  autoconf.sh: 13: gperf: not found.

You will need to install the ``autoconf`` and ``gperf`` tools before you can
continue.

The other way to get the source code is to download a released `tar/zip` file::

  % tar -xvzf v13_0.tar.gz
  or
  % unzip v13_0.zip

See the build instructions for your operation system below to know what to do
next. Though first determine if there are any extra configuration option you
may need.

Icarus Specific Configuration Options
-------------------------------------

Icarus takes many of the standard configuration options and those will not be
described here. The following are specific to Icarus::

  --enable-suffix[=suffix]

This option allows the user to build Icarus with a default suffix or when
provided a user defined suffix. Older stable releases have this flag on by
default e.g.(V0.8 by default will build with a "-0.8" suffix). All versions
have an appropriate default suffix ("-<base_version>").

All programs or directories are tagged with this suffix. e.g.(iverilog-0.8,
vvp-0.8, etc.). The output of iverilog will reference the correct run time
files and directories. The run time will check that it is running a file with
a compatible version e.g.(you can not run a V0.9 file with the V0.8 run
time).::

  --with-valgrind

This option adds extra memory cleanup code and pool management code to allow
better memory leak checking when valgrind is available. This option is not
needed when checking for basic errors with valgrind and should not be used if
you just intend to use ``iverilog`` as a simulator. ::

  --enable-libvvp

The vvp program is built as a small stub linked to a shared library,
libvvp.so, that may be linked with other programs so that they can host
a vvp simulation. ::

  --enable-libveriuser

PLI version 1 (the ACC and TF routines) were deprecated in IEEE 1364-2005.
These are supported in Icarus Verilog by the libveriuser library and cadpli
module. Starting with V13, these will only be built if this option is used.

Compiling on Linux/Unix
-----------------------

Note: For a gcc compile you will need to install ``bison``, ``flex``, ``g++``,
``gcc`` and preferably `bz2`, `zlib` and `readline` development packages. The
`bz2` and `zlib` development packages are required for the non-VCD waveform
dumpers and the `readline` development package is needed to enable better
terminal control in the ``vvp`` interactive mode.

If you are only compiling one variant then you can compile directly in the
source tree. If you need multiple variants (optimized, debugging, multiple
compilers) then it is recommended you compile each in their own directory.

For multiple variants create a directory for each of the variants you intend
to create and in each run the following steps, adjusting the options in the
configure stage to get the functionality you want. For a single build you can
either build it with the source or in a separate build directory.

The following is from a Ubuntu 22.04 machine using gcc (version 11.4)::

  % mkdir gcc
  % cd gcc
  or
  % cd iverilog

You can also use ``clang/clang++``. I usual build optimized version for
normal use and reserve debugging options for a valgrind or a separate
debugging build. Make sure you have `sudo` permission if you are using a
system prefix area, otherwise you need to use some place you have
permission to install (e.g. ~/).::

  % env CFLAGS=-O2 CXXFLAGS=-O2 LDFLAGS=-s CC=gcc CXX=g++ ../iverilog/configure --enable-suffix=-gcc --prefix=/usr/local

This will generate the following (with some inline comments)::

  checking build system type... x86_64-pc-linux-gnu
  checking host system type... x86_64-pc-linux-gnu
  checking for gcc... gcc
  checking whether the C compiler works... yes
  ...
  checking for gperf... gperf       # required for git builds
  checking for man... man           # you likely want manual pages
  checking for ps2pdf... ps2pdf
  checking for groff... groff
  checking for git... git           # required for git builds
  checking for flex... flex         # required
  checking for bison... bison       # required
  ...
  checking for tputs in -ltermcap... yes
  checking for readline in -lreadline... yes
  checking for add_history in -lreadline... yes
  checking for readline/readline.h... yes
  checking for readline/history.h... yes           # you likely want this
  ...
  checking for pthread_create in -lpthread... yes
  checking for gzwrite in -lz... yes
  checking for gzwrite in -lz... (cached) yes
  checking for BZ2_bzdopen in -lbz2... yes
  checking for BZ2_bzdopen in -lbz2... (cached) yes     # you want these for fst dumping
  ...
  <Create all the parameterized Makefile and header files>

Usually if ``configure`` fails there is some required dependency missing. I
usually review all the output to make sure it makes sense (e.g. I requested
``gcc`` and that's what is being used, other things match my expectation). If
all the waveform dumpers are not enabled there could be a few test failures.

Next we need to compile the code. Note: make sure you are using GNU make.
It may be named gmake (e.g. GhostBSD)::

  % make check >& make.log

This is for a tcsh/csh shell. Bash/fish/zsh use ``&>`` instead of ``>&``.
Once this has completed check the make.log for any errors. There should not
be any! I also check for warnings. There are often some related to the
output from bison. For example::

  From: ./parse.cc
  parse.cc:9462:18: warning: missing initializer for member ‘vlltype::lexical_pos’ [-Wmissing-field-initializers]
   9462 |   = { 1, 1, 1, 1 }
        |                  ^
  parse.cc:9462:18: warning: missing initializer for member ‘vlltype::text’ [-Wmissing-field-initializers]

and::

  From: ./vvp/parse.cc
  parse.cc:3242: warning: suspicious sequence in the output: m4_type [-Wother]
  parse.cc:3248: warning: suspicious sequence in the output: m4_type [-Wother]

Are common, but benign warnings. Different compilers or compiler versions may
have other warnings.

The expected last few lines of the make.log file and these indicate everything
should be working as expected are::

  ...
  driver/iverilog -B. -BMvpi -BPivlpp -tcheck -ocheck.vvp ../iverilog/examples/hello.vl
  vvp/vvp -M- -M./vpi ./check.vvp | grep 'Hello, World'
  Hello, World

If everything is good to this point and you are installing into a system
prefix; install using ``sudo`` as shown below. If you are installing into a
personal location skip the ``sudo``::

  % sudo make install

Now you should verify the regression test suite is working as expected::

  % cd ../iverilog/ivtest
  % ./vvp_reg.pl --suffix=-gcc

This is the original test script and should give no failures::

  Running compiler/VVP tests for Icarus Verilog version: 13, suffix: -gcc.
  ----------------------------------------------------------------------------
              macro_with_args: Passed.
                         mcl1: Passed.
                        pr622: Passed.
                        pr639: Passed.
                           ...
                     ssetclr2: Passed.
                     ssetclr3: Passed.
             synth_if_no_else: Passed.
                  ufuncsynth1: Passed.
  ============================================================================
  Test results:
    Total=3018, Passed=3013, Failed=0, Not Implemented=2, Expected Fail=3

Next run the new test script::

  % ./vvp_reg.py --suffix=-gcc

This should also give no failures::

  Running compiler/VVP tests for Icarus Verilog version: 13, suffix: -gcc
  Using list(s): regress-vvp.list
  ----------------------------------------------------------------------------
                      always4A: Passed - CE.
                      always4B: Passed - CE.
                       analog1: Not Implemented.
                       analog2: Not Implemented.
                            ...
                vvp_quiet_mode: Passed.
               warn_opt_sys_tf: Passed - EF.
                         wreal: Passed.
              writemem-invalid: Passed - EF.
  ============================================================================
  Test results: Ran 284, Failed 0.

Finally you can check that the VPI is working properly using::

  % ./vpi_reg.pl --suffix=-gcc

The output for this should have no failures::

  Running VPI tests for Icarus Verilog version: 13, suffix: -gcc.
  ----------------------------------------------------------------------------
            br_gh59: Passed.
           br_gh73a: Passed.
           br_gh73b: Passed.
           br_gh117: Passed.
                 ...
   value_change_cb2: Passed.
   value_change_cb3: Passed.
   value_change_cb4: Passed.
        vpi_control: Passed.
  ============================================================================
  Test results: Total=77, Passed=77, Failed=0, Not Implemented=0

You can uninstall everything using the following. If needed skip the ``sudo``
as described in the install description above.::

  % sudo make uninstall

You can cleanup the compile directory using::

  % make clean
  or
  % make distclean

The first just cleans up just the compiled files, etc. The later cleans up
the compiled file along with all the files generated in the ``configure``
phase.

Note that "rpm" packages of binaries for Linux are typically configured with
"--prefix=/usr" per the Linux File System Standard.

Make sure you have a recent version of flex otherwise you will get an error
when parsing lexor.lex.

Compiling on Macintosh OS X
---------------------------

Since Mac OS X is a BSD flavor of Unix, you can install Icarus Verilog from
source using the procedure described above. You need to install the Xcode
software, which includes the C and C++ compilers for Mac OS X. The package is
available for free download from Apple's developer site. Once Xcode is
installed, you can build Icarus Verilog in a terminal window just like any
other Unix install.

For versions newer than 10.3 the GNU Bison tool (packaged with Xcode) needs to
be updated to version 3. ::

  brew install bison
  echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.bash_profile

Icarus Verilog is also available through the Homebrew package manager: "brew
install icarus-verilog".

Cross-Compiling for Windows
---------------------------

The `Cygwin` and `MSYS2` environments can compile Icarus Verilog as described
above for `Linux/Unix`. There is a `MSYS2` build recipe which can be found in
the `msys2/` directory. The accompanying README file provides further details.
`MSYS2` is typically preferred over `Cygwin` since ``GTKWave`` and Icarus
Verilog are both provided as pre-compiled packages.

What follows are older instructions for building Icarus Verilog binaries for
Windows using mingw cross compiler tools on Linux.

To start with, you need the mingw64-cross-* packages for your linux
distribution, which gives you the x86_64-w64-mingw32-* commands
installed on your system. Installing the cross environment is outside
the scope of this writeup.

First, configure with this command::

  $ ./configure --host=x86_64-w64-mingw32

This generates the Makefiles needed to cross compile everything with
the mingw32 compiler. The configure script will generate the command
name paths, so long as commands line x86_64-w64-mingw32-gcc
et. al. are in your path.

Next, compile with the command::

  $ make

The configure generated the cross compiler flags, but there are a few
bits that need to be compiled with the native compiler. (version.exe
for example is used by the build process but is not installed.) The
configure script should have gotten all that right.
