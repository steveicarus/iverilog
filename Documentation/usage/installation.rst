
Installation Guide
==================

Icarus Verilog may be installed from source code, or from pre-packaged binary
distributions. If you don't have need for the very latest, and prepackaged
binaries are available, that would be the best place to start.

Installation From Source
------------------------

Icarus is developed for Unix-like environments but can also be compiled on
Windows systems using the Cygwin environment or MinGW compilers. The following
instructions are the common steps for obtaining the Icarus Verilog source,
compiling and installing. Note that there are precompiled and/or prepackaged
versions for a variety of systems, so if you find an appropriate packaged
version, then that is the easiest way to install.

The source code for Icarus is stored under the git source code control
system. You can use git to get the latest development head or the latest of a
specific branch. Stable releases are placed on branches, and in particular v11
stable releases are on the branch "v11-branch" To get the development version
of the code follow these steps::

  % git config --global user.name "Your Name Goes Here"
  % git config --global user.email you@yourpublicemail.example.com
  % git clone https://github.com/steveicarus/iverilog.git

The first two lines are optional and are used to tell git who you are. This
information is important if/when you submit a patch. We suggest that you add
this information now so you don't forget to do it later. The clone will create
a directory, named iverilog, containing the source tree, and will populate
that directory with the most current source from the HEAD of the repository.

Change into this directory using::

  % cd iverilog

Normally, this is enough as you are now pointing at the most current
development code, and you have implicitly created a branch "master" that
tracks the development head. However, If you want to actually be working on
the v11-branch (the branch where the latest v11 patches are) then you checkout
that branch with the command::

  % git checkout --track -b v11-branch origin/v11-branch

This creates a local branch that tracks the v11-branch in the repository, and
switches you over to your new v11-branch. The tracking is important as it
causes pulls from the repository to re-merge your local branch with the remote
v11-branch. You always work on a local branch, then merge only when you
push/pull from the remote repository.

Now that you've cloned the repository and optionally selected the branch you
want to work on, your local source tree may later be synced up with the
development source by using the git command::

  % git pull

The git system remembers the repository that it was cloned from, so you don't
need to re-enter it when you pull.

Finally, configuration files are built by the extra step::

  % sh autoconf.sh

The source is then compiled as appropriate for your system. See the specific
build instructions below for your operation system for what to do next.

You will need autoconf and gperf installed in order for the script to work.
If you get errors such as::

  Autoconf in root...
  autoconf.sh: 10: autoconf: not found
  Precompiling lexor_keyword.gperf
  autoconf.sh: 13: gperf: not found.

You will need to install download and install the autoconf and gperf tools.

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
time). ::

  --with-valgrind

This option adds extra memory cleanup code and pool management code to allow
better memory leak checking when valgrind is available. This option is not
need when checking for basic errors with valgrind.

Compiling on Linux/Unix
-----------------------

(Note: You will need to install bison, flex, g++ and gcc) This is probably the
easiest case. Given that you have the source tree from the above instructions,
the compile and install is generally as simple as::

  % ./configure
  % make
  (su to root)
  # make install

The "make install" typically needs to be done as root so that it can install
in directories such as "/usr/local/bin" etc. You can change where you want to
install by passing a prefix to the "configure" command::

  % ./configure --prefix=/my/special/directory

This will configure the source for eventual installation in the directory that
you specify. Note that "rpm" packages of binaries for Linux are typically
configured with "--prefix=/usr" per the Linux File System Standard.

Make sure you have the latest version of flex otherwise you will get an error
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

Compiling for Windows
---------------------

These are instructions for building Icarus Verilog binaries for
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

There is also a MSYS2 build recipe which you can find under `msys2/` in the repository.
