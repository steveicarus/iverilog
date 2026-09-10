
vhdlpp Command Line Flags
=========================

* -D <token>

  Debug flags. The token can be:

  * yydebug | no-yydebug

  * entities=<path>

* -L <path>

  Library path. Add the directory name to the front of the library
  search path. The library search path is initially empty.

* -V

  Display version on stdout

* -v

  Verbose: Display version on stderr, and enable verbose messages to
  stderr.

* -w <path>

  Work path. This is the directory where the working directory is.


Library Format
--------------

The vhdlpp program stores libraries as directory that contain
packages. The name of the directory (in lower case) is the name of the
library as used on the "import" statement. Within that library, there
are packages in files named <foo>.pkg. For example::

    <directory>/...
       sample/...
         test1.pkg
	 test2.pkg
       bar/...
         test3.pkg

Use the "+vhdl-libdir+<directory>" record in a config file to tell
Icarus Verilog that <directory> is a place to look for libraries. Then
in your VHDL code, access packages like this::

    library sample;
    library bar;
    use sample.test1.all;
    use bar.test3.all;

The \*.pkg files are just VHDL code containing only the package with
the same name. When Icarus Verilog encounters the "use <lib>.<name>.*;"
statement, it looks for the <name>.pkg file in the <lib> library and
parses that file to get the package header declared therein.
