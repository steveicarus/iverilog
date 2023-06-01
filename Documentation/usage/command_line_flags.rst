

iverilog Command Line Flags
===========================

The iverilog command is the compiler/driver that takes the Verilog input and
generates the output format, whether the simulation file or synthesis
results. This information is at least summarized in the iverilog man page
distributed in typical installations, but here we try to include more detail.

General
-------

These flags affect the general behavior of the compiler.

* -c <cmdfile>

  This flag selects the command file to use. The command file is an
  alternative to writing a long command line with a lot of file names and
  compiler flags. See the Command File Format page for more information.

* -d <flag>

  Enable compiler debug output. These are aids for debugging Icarus Verilog,
  and this flag is not commonly used.
  The flag is one of these debug classes:

  * scope
  * eval_tree
  * elaborate
  * synth2

* -g <generation flag>
  the generation is the compiler language, and specifies the language and
  extensions to use during the compile. The language level can be selected
  by a major level selector, and by controlling various features. Various
  "-g" flags can be compined. For example, to get Verilog 2001 without
  specify supoprt, use "-g2001 -gno-specify".

  The supported flags are:

  * 1995

    This flag enables the IEEE1364-1995 standard.

  * 2001

    This flag enables the IEEE1364-2001 standard.

  * 2001-noconfig

    This flag enables the IEEE1364-2001 standard with config file support
    disabled. This eliminates the config file keywords from the language and
    so helps some programs written to older 2001 support compile.

  * 2005
    This flag enables the IEEE1364-2005 standard. This is default enabled
    after v0.9.

  * 2009
    This flag enables the IEEE1800-2009 standard, which includes
    SystemVerilog. The SystemVerilog support is not present in v0.9 and
    earlier. It is new to git master as of November 2009. Actual SystemVerilog
    support is ongoing.

  * 2012

    This flag enables the IEEE1800-2012 standard, which includes
    SystemVerilog.

  * verilog-ams

    This flag enables Verilog-AMS features that are supported by Icarus
    Verilog. (This is new as of 5 May 2008.)

  * assertions/supported-assertions/no-assertions

    Enable or disable SystemVerilog assertions. When enabled, assertion
    statements are elaborated. When disabled, assertion statements are parsed
    but ignored. The supported-assertions option only enables assertions that
    are currently supported by the compiler.

  * specify/no-specify

    Enable or disable support for specify block timing controls. When
    disabled, specify blocks are parsed but ignored. When enabled, specify
    blocks cause timing path and timing checks to be active.

  * std-include/no-std-include

    Enable or disable the search of a standard installation include directory
    after all other explicit include directories. This standard include
    directory is a convenient place to install standard header files that a
    Verilog program may include.

  * relative-include/no-relative-include

    Enable or disable adding the local files directory to the beginning of the
    include file search path. This allows files to be included relative to the
    current file.

  * xtypes/no-xtypes

    Enable or disable support for extended types. Enabling types allows for
    new types and type syntax that are Icarus Verilog extensions.

  * io-range-error/no-io-range-error

    When enabled the range for a port and any associated net declaration must
    match exactly. When disabled a scalar port is allowed to have a net
    declaration with a range (obsolete usage). A warning message will be
    printed for this combination. All other permutations are still considered
    an error.

  * strict-ca-eval/no-strict-ca-eval

    The standard requires that if any input to a continuous assignment
    expression changes value, the entire expression is re-evaluated. By
    default, parts of the expression that do not depend on the changed input
    value(s) are not re-evaluated. If an expression contains a call to a
    function that doesn't depend solely on its input values or that has side
    effects, the resulting behavior will differ from that required by the
    standard. Enabling strict-ca-eval will force standard compliant behavior
    (with some loss in performance).

  * strict-expr-width/no-strict-expr-width

    Enable or disable strict compliance with the standard rules for
    determining expression bit lengths. When disabled, the RHS of a parameter
    assignment is evaluated as a lossless expression, as is any expression
    containing an unsized constant number, and unsized constant numbers are
    not truncated to integer width.

  * shared-loop-index/no-shared-loop-index

    Enable or disable the exclusion of for-loop control variables from
    implicit event_expression lists. When enabled, if a for-loop control
    variable (loop index) is only used inside the for-loop statement, the
    compiler will not include it in an implicit event_expression list it
    calculates for that statement or any enclosing statement. This allows the
    same control variable to be used in multiple processes without risk of
    entering an infinite loop caused by each process triggering all other
    processes that use the same variable. For strict compliance with the
    standards, this behaviour should be disabled.

* -i

  Ignore missing modules. Normally it is an error if a module instantiation
  refers to an undefined module. This option causes the compiler to skip over
  that instantiation. It will also stop the compiler returning an error if
  there are no top level modules. This allows the compiler to be used to check
  incomplete designs for errors.

  NOTE: The "-i" flag was added in v11.0.

* -L <path>

  Add the specified directory to the path list used to locate VPI modules. The
  default path includes only the install directory for the system.vpi module,
  but this flag can add other directories. Multiple paths are allowed, and the
  paths will be searched in order.

  NOTE: The "-L" flag was added in v11.0.

* -l <path>

  Add the specified file to the list of source files to be compiled, but mark
  it as a library file. All modules contained within that file will be treated
  as library modules, and only elaborated if they are instantiated by other
  modules in the design.

  NOTE: The "-l" flag is new as of 2 October 2016. It will become available in
  releases and snapshots made after that date.

* -M<mode>=<path>

  Write into the file specified by path a list of files that contribute to the
  compilation of the design.

  If _mode_ is *all* or *prefix*, this includes files that are included by
  include directives and files that are automatically loaded by library
  support as well as the files explicitly specified by the user.

  If _mode_ is *include*, only files that are included by include directives
  are listed.

  If _mode_ is *module*, only files that are specified by the user or that are
  automatically loaded by library support are listed. The output is one file
  name per line, with no leading or trailing space.

  If _mode_ is *prefix*, files that are included by include directives are
  prefixed by "I " and other files are prefixed by "M ".

* -m<module>

  Add this module to the list of VPI modules to be loaded by the
  simulation. Many modules can be specified, and all will be loaded, in the
  order specified. The system module is implicit and always included (and
  loaded last).

  If the specified name includes at least one directory character, it is
  assumed to be prefixed by the path to the module, otherwise the module is
  searched for in the paths specified by preceding -L options, and if not
  found there, in the iverilog base directory.

  NOTE: The "-m" flag was added in v11.0.

* -o <path>

  Specify the output file. The <path> is the name of the file to hold the
  output. The default is "a.out".

* -S

  Activate synthesis. This flag tells the compiler to do what synthesis it can
  do before calling the code generator. This flag is rarely used explicitly,
  and certain code generators will implicitly enable this flag.

* -u

  Treat each source file as a separate compilation unit (as defined in
  SystemVerilog). If compiling for an IEEE1364 generation, this will just
  reset all compiler directives (including macro definitions) before each new
  file is processed.

  NOTE: The "-u" flag was added in v11.0.

* -v

  Be verbose. Print copyright information, progress messages, and some timing
  information about various compilation phases.

  (New in snapshots after 2014-12-16) If the selected target is vvp, the -v
  switch is appended to the shebang line in the compiler output file, so
  directly executing the compiler output file will turn on verbose messages in
  vvp. This extra verbosity can be avoided by using the vvp command to
  indirectly execute the compiler output file.

* -V

  Print the version information. This skips all compilation. Just print the
  version information, including version details for the various components of
  the compiler.

* -R

  Print the runtime paths of the compiler. This can be useful to find, e.g.,
  the include path of vpi_user.h.

* -W<warning class>

  Enable/disable warnings. All the warning types (other then "all") can be
  prefixed with no- to disable that warning.

  * all

    This enables almost all of the available warnings. More specifically, it
    enables these warnings::

      -Wanachronisms
      -Wimplicit
      -Wimplicit-dimensions
      -Wmacro-replacement
      -Wportbind
      -Wselect-range
      -Wtimescale
      -Wsensitivity-entire-array

  * anachronisms

    This enables warnings for use of features that have been deprecated or
    removed in the selected generation of the Verilog language.

  * implicit

    This enables warnings for creation of implicit declarations. For example,
    if a scalar wire X is used but not declared in the Verilog source, this
    will print a warning at its first use.

  * implicit-dimensions

    This enables warnings for the case where a port declaration or a var/net
    declaration for the same name is missing dimensions. Normally, Verilog
    allows you to do this (the undecorated declaration gets its dimensions
    form the decorated declaration) but this is no longer common, and some
    other tools (notable Xilix synthesizers) do not handle this correctly.

    This flag is supported in release 10.1 or master branch snapshots after
    2016-02-06.

  * macro-redefinition

    This enables warnings when a macro is redefined, even if the macro text
    remains the same.

    NOTE: The "macro-redefinition" flag was added in v11.0.

  * macro-replacement

    This enables warnings when a macro is redefined and the macro text
    changes. Use no-macro-redefinition to disable this,

    NOTE: The "macro-replacement" flag was added in v11.0.

  * portbind

    This enables warnings for ports of module instantiations that are not
    connected properly, but probably should be. Dangling input ports, for
    example, will generate a warning.

  * select-range

    This enables warnings for constant out-of-bound selects. This includes
    partial or fully out-of-bound select as well as a select containing a 'bx
    or 'bz in the index.

  * timescale

    This enables warnings for inconsistent use of the timescale directive. It
    detects if some modules have no timescale, or if modules inherit timescale
    from another file. Both probably mean that timescales are inconsistent,
    and simulation timing can be confusing and dependent on compilation order.

  * infloop

    This enables warnings for always statements that may have runtime infinite
    loops (i.e. has paths with zero or no delay). This class of warnings is
    not included in -Wall and hence does not have a no- variant. A fatal error
    message will always be printed when the compiler can determine that there
    will definitely be an infinite loop (all paths have no or zero delay).

    When you suspect an always statement is producing a runtine infinite loop,
    use this flag to find the always statements that need to have their logic
    verified. it is expected that many of the warnings will be false
    positives, since the code treats the value of all variables and signals as
    indeterninite.

  * sensitivity-entire-vector

    This enables warnings for when a part select with an "always @*" statement
    results in the entire vector being added to the implicit sensitivity
    list. Although this behavior is prescribed by the IEEE standard, it is not
    what might be expected and can have performance implications if the vector
    is large.

  * sensitivity-entire-array

    This enables warnings for when a word select with an "always @*" statement
    results in the entire array being added to the implicit sensitivity
    list. Although this behavior is prescribed by the IEEE standard, it is not
    what might be expected and can have performance implications if the array
    is large.

  * floating-nets

    This enables warnings for nets that are present but have no drivers.

    This flag was added in version 11.0 or later (and is in the master branch
    as of 2015-10-01).

* -y<libdir>

  Append the directory to the library module search path. When the compiler
  finds an undefined module, it looks in these directories for files with the
  right name.

* -Y<suf>

  Appends suf to the list of file extensions that are used to resolve an
  undefined module to a file name. Should be specified before any -y flag. For
  example, this command::

    % iverilog -Y .sv -y sources src.v

  will try to resolve any undefined module m by looking into the directory
  sources and checking if there exist files named m.v or m.sv.


Preprocessor Flags
------------------

These flags control the behavior of the preprocessor. They are similar to
flags for the typical "C" compiler, so C programmers will find them familiar.

* -E

  This flag is special in that it tells the compiler to only run the
  preprocessor. This is useful for example as a way to resolve preprocessing
  for other tools. For example, this command::

    % iverilog -E -ofoo.v -DKEY=10 src1.v src2.v

  runs the preprocessor on the source files src1.v and src2.v and produces the
  single output file foo.v that has all the preprocessing (including header
  includes and ifdefs) processed.

* -D<macro>

  Assign a value to the macro name. The format of this flag is one of::

    -Dkey=value
    -Dkey

  The key is defined to have the given value. If no value is given, then it is
  assumed to be "1". The above examples are the same as these defines in
  Verilog source::

    `define key value
    `define key

* -I<path>

  Append directory <path> to list of directories searched for Verilog include
  files. The -I switch may be used many times to specify several directories
  to search, the directories are searched in the order they appear on the
  command line.

Elaboration Flags
-----------------

These are flags that pass information to the elaboration steps.

* -P<symbol>=<value>

  Define a parameter using the defparam behavior to override a parameter
  values. This can only be used for parameters of root module instances.

* -s <topmodule>

  Specify the top level module to elaborate. Icarus Verilog will by default
  choose modules that are not instantiated in any other modules, but sometimes
  that is not sufficient, or instantiates too many modules. If the user
  specifies one or more root modules with "-s" flags, then they will be used
  as root modules instead.

* -Tmin, -Ttyp, -Tmax

  Select the timings to use. The Verilog language allows many timings to be
  specified as three numbers, min:typical:max, but for simulation you need to
  choose which set to use. The "-Tmin" flag tells the compiler to at
  elaboration time choose "min" times. The default is "-Ttyp".

Target Flags
------------
