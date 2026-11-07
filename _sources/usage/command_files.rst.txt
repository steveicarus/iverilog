
Command File Format
===================

The basic format of a command file is one source file or compiler argument per
line. Command files may also have comments of various form, and options for
controlling the compiler.

Comments
--------

Lines that start with a "#" character are comments. All text after the "#"
character, is ignored.

The "//" character sequence also starts a comment that continues to the end of
the line.

The "/\*" and "\*/" character sequences surround multi-line comments. All the
text between the comment start and comment end sequences is ignored, even when
that text spans multiple lines. This style of comment does not nest, so a "/\*"
sequence within a multi-line comment is probably an error.

Plus-args
---------

Outside of comments, lines that start with a "+" character are compiler
arguments. These are called plusargs but they are not the same as extended
arguments passed to the "vvp" command. The supported plusargs are definitively
listed in the iverilog manual page.

The plusargs lines are generally "+<name>+..." where the name is the name of
an switch, and the arguments are separated by "+" characters, as in::

  +libext+.v+.V+.ver

With plusargs lines, the "+" character separates tokens, and not white space,
so arguments, which may include file paths, may include spaces. A plusarg line
is terminated by the line end.

The line in the command file may also be a "-y" argument. This works exactly
the same as the::

  -y <path>

argument to the compiler; it declares a library directory. The "-y" syntax is
also a shorthand for the "+libdir" plusarg, which is a more general form::

  +libdir+<path>...

File Names
----------

Any lines that are not comments, compiler arguments or plusargs are taken by
the compiler to be a source file. The path can contain any characters (other
then comment sequences) including blanks, although leading and trailing white
space characters are stripped. The restriction of one file name per line is in
support of operating systems that can name files any which way. It is not
appropriate to expect white spaces to separate file names.

Variable Substitution
---------------------

The syntax "$(name)" is a variable reference, and may be used anywhere within
filenames or directory names. The contents of the variable are read from the
environment and substituted in place of the variable reference. In Windows,
these environment variables are the very same variables that are set through
the Control Panel->System dialog box, and in UNIX these variables are
environment variables as exported by your shell.

Variables are useful for giving command files some installation
independence. For example, one can import a vendor library with the line::

  -y $(VENDOR)/verilog/library

in the command file, and the next programmer will be able to use this command
file without editing it to point to the location of VENDOR on his
machine. Note the use of forward slashes as a directory separator. This works
even under Windows, so always use forward slashes in file paths and Windows
and UNIX users will be able to share command files.

An Example
----------

This sample::

  # This is a comment in a command file.
  # The -y statement declares a library
  # search directory
  -y $(PROJ_LIBRARY)/prims
  #
  # This plusarg tells the compiler that
  # files in libraries may have .v or .vl
  # extensions.
  +libext+.v+.vl
  #
  main.v // This is a source file
  #
  # This is a file name with blanks.
  C:/Project Directory/file name.vl

is a command file that demonstrates the major syntactic elements of command
files. It demonstrates the use of comments, variables, plusargs and file
names. It contains a lot of information about the hypothetical project, and
suggests that command files can be used to describe the project as a whole
fairly concisely.

The syntax of command files is rich enough that they can be used to document
and control the assembly and compilation of large Verilog programs. It is not
unusual to have command files that are hundreds of lines long, although
judicious use of libraries can lead to very short command files even for large
designs. It is also practical to have different command files that pull
together combinations of sources and compiler arguments to make different
designs from the same Verilog source files.

Summary
-------

Given the above description of the command file format, the following is a
list of the special records with their meaning.

* +libdir+*dir-path*

  Specify directories to be searched for library modules. The *dir-path* can
  have multiple directories, separated by "+" characters.

* +libdir-nocase+dir-path

  This is the same as "+libdir+", but when searching "nocase" libraries for
  module files, case will not be taken as significant. This is useful when the
  library is on a case insensitive file system.

* +libext+*suffix-string*

  Declare the suffix strings to use when searching library directories for
  Verilog files. The compiler may test a list of suffix strings to support a
  variety of naming conventions.

* -y dir-path

  This is like "+libdir+" but each line takes only one path. Like "+libdir+"
  there can be multiple "-y" records to declare multiple library
  directories. This is similar to the "-y" flag on the iverilog command line.

* -v *file-name* or -l *file-name*

  This declares a library file. A library file is just like any other Verilog
  source file, except that modules declared within it are not implicitly
  possible root modules.

  NOTE: The "-l" alias is new as of 2 October 2016. It will become available
  in releases and snapshots made after that date.

* +incdir+*include-dir-path*

  Declare a directory or list of directories to search for files included by
  the "include" compiler directive. The directories are searched in
  order. This is similar to the "-I" flag on the iverilog command line.

* +define+*name=value*

  Define the preprocessor symbol "name" to have the string value "value". If
  the value (and the "=") are omitted, then it is assumed to be the string
  "1". This is similar to the "-D" on the iverilog command line.

* +timescale+*units/precision*

  Define the default timescale. This is the timescale that is used if there is
  no other timescale directive in the Verilog source. The compiler default
  default is "+timescale+1s/1s", which this command file setting can
  change. The format of the units/precision is the same as that for the
  timescale directive in the verilog source.

* +toupper-filename

  This token causes file names after this in the command file to be translated
  to uppercase. this helps with situations where a directory has passed
  through a DOS machine (or a FAT file system) and in the process the file
  names become munged. This is not meant to be used in general, but only in
  emergencies.

* +tolower-filename

  The is the lowercase version of "+toupper-filename".

* +parameter+*name=value*

  This token causes the compiler to override a parameter value for a top-level
  module. For example, if the module main has the parameter WIDTH, set the
  width like this "+parameter+main.WIDTH=5". Note the use of the complete
  hierarchical name. This currently only works for parameters defined in root
  (top level) modules and a defparam may override the command file value.

* +vhdl-work+*path*

  When compiling VHDL, this token allows control over the directory to use for
  holding working package declarations. For example, "+vhdl-work+workdir" will
  cause the directory "workdir" to be used as a directory for holding working
  working copies of package headers.
