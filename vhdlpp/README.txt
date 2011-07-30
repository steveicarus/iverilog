
vhdlpp COMMAND LINE FLAGS:

-D <token>
  Debug flags. The token can be:

  * yydebug | no-yydebug

  * entities=<path>

-L <path>
  Library path. Add the directory name to the front of the library
  search path. The library search path is initially empty.

-V
  Display version on stdout

-v
  Verbose: Display version on stderr, and enable verbose messages to
  stderr.

-w <path>
  Work path. This is the directory where the working directory is.


LIBRARY FORMAT:

The vhdlpp program stores libraries as directory that contain
packages. The name of the directory (in lower case) is the name of the
library as used on the "import" statement. Within that library, there
are packages in files named <foo>.pkg.
