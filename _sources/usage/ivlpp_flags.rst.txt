
IVLPP - IVL Preprocessor
========================

The ivlpp command is a Verilog preprocessor that handles file
inclusion and macro substitution. The program runs separate from the
actual compiler so as to ease the task of the compiler proper, and
provides a means of preprocessing files off-line.


USAGE:

	ivlpp [options] <file>

The <file> parameter is the name of the file to be read and
preprocessed. The resulting output is sent to standard output. The
valid options include:

* -Dname[=value]

	    Predefine the symbol `name` to have the specified
	    value. If the value is not specified, then `1` is
	    used. This is mostly of use for controlling conditional
	    compilation.

	    This option does *not* override existing \`define
	    directives in the source file.

* -F <path>

	    Read ivlpp options from a FLAGS FILE. This is not the same
	    as a file list. This file contains flags, not source
	    files. There may be multiple flags files.

* -f <path>

	    Read ivlpp input files from a file list. There can be no
	    more than one file list.

* -I <dir>

	    Add a directory to the include path. Normally, only "." is
	    in the search path. The -I flag causes other directories
	    to be searched for a named file. There may be as many -I
	    flags as needed.

* -L

	    Generate \`line directives. The ivl compiler understands
	    these directives and uses them to keep track of the
	    current line of the original source file. This makes error
	    messages more meaningful.

* -o <file>

	    Send the output to the named file, instead of to standard
	    output.

* -v

	    Print version and copyright information before processing
	    input files.

* -V

	    Print version and copyright information, then exit WITHOUT
	    processing any input files.

Flags File
----------

A flags file contains flags for use by ivlpp. This is a convenient way
for programs to pass complex sets of flags to the ivlpp program.

Blank lines and lines that start with "#" are ignored. The latter can
be used as comment lines. All other lines are flag lines. Leading and
trailing white space are removed before the lines are interpreted.

Other lines have the simple format::

  <key>:<value>

The colon character separates a key from the value. The supported
keys, with their corresponding values, are:

* D:name=<value>

      This is exactly the same as the "-Dname=<value>" described above.

* I:<dir>

      This is exactly the same as "-I<dir>".

* relative include:<flag>

      The <flag> can be "true" or "false". This enables "relative
      includes" nesting behavior.

* vhdlpp:<path>

      Give the path to the vhdlpp program. This program is used to
      process VHDL input files.

Locating Included Files
-----------------------

The ivlpp preprocessor implements the \`include directives by
substituting the contents of the included file in place of the line
with the \`include directive. The name that the programmer specifies is
a file name. Normally, the preprocessor looks in the current working
directory for the named file. However, the `-I` flags can be used to
specify a path of directories to search for named include files. The
current directory will be searched first, followed by all the include
directories in the order that the -I flag appears.

The exception to this process is include files that have a name that
starts with the '/' character. These file names are `rooted names`
and must be in the rooted location specified.


Generated Line Directives
-------------------------

Compilers generally try to print along with their error messages the
file and line number where the error occurred. Icarus Verilog is no
exception. However, if a separate preprocessor is actually selecting
and opening files, then the line numbers counted by the compiler
proper will not reflect the actual line numbers in the source file.

To handle this situation, the preprocessor can generate line
directives. These directives are lines of the form::

	`line <num> <name> <level>

where <name> is the file name in double-quotes and <num> is the line
number in the file. The parser changes the filename and line number
counters in such a way that the next line is line number <num> in
the file named <name>. For example::

	`line 6 "foo.vl" 0
	// I am on line 6 in file foo.vl.

The preprocessor generates a \`line directive every time it switches
files. That includes starting an included file (\`line 1 "foo.vlh" 1) or
returning to the including file.

