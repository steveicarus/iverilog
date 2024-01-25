# The ICARUS Verilog Compilation System

Copyright 2000-2019 Stephen Williams

<details>
<summary><h2>Table of Contents</h2></summary>

1. [What is ICARUS Verilog?](#what-is-icarus-verilog)
2. [Building/Installing Icarus Verilog From Source](#buildinginstalling-icarus-verilog-from-source)
	- [Compile Time Prerequisites](#compile-time-prerequisites)
	- [Compilation](#compilation)
	- [(Optional) Testing](#optional-testing)
	- [Installation](#installation)
3. [How Icarus Verilog Works](#how-icarus-verilog-works)
	- [Preprocessing](#preprocessing)
	- [Parse](#parse)
	- [Elaboration](#elaboration)
	- [Optimization](#optimization)
	- [Code Generation](#code-generation)
	- [Attributes](#attributes)
4.	[Running iverilog](#running-iverilog)
	- [Examples](#examples)
5. [Unsupported Constructs](#unsupported-constructs)
6. [Nonstandard Constructs or Behaviors](#nonstandard-constructs-or-behaviors)
	- [Builtin system functions](#builtin-system-functions)
	- [Preprocessing Library Modules](#preprocessing-library-modules)
	- [Width in %t Time Formats](#width-in-t-time-formats)
	- [vpiScope iterator on vpiScope objects](#vpiscope-iterator-on-vpiscope-objects)
	- [Time 0 Race Resolution](#time-0-race-resolution)
	- [Nets with Types](#nets-with-types)
7. [Credits](#credits)

</details>

## What is ICARUS Verilog?

Icarus Verilog is intended to compile ALL of the Verilog HDL, as
described in the IEEE-1364 standard. Of course, it's not quite there
yet. It does currently handle a mix of structural and behavioural
constructs. For a view of the current state of Icarus Verilog, see its
home page at https://steveicarus.github.io/iverilog/.

Icarus Verilog is not aimed at being a simulator in the traditional
sense, but a compiler that generates code employed by back-end
tools.

> For instructions on how to run Icarus Verilog, see the `iverilog` man page.


## Building/Installing Icarus Verilog from Source

If you are starting from the source, the build process is designed to be
as simple as practical. Someone basically familiar with the target
system and C/C++ compilation should be able to build the source
distribution with little effort. Some actual programming skills are
not required, but helpful in case of problems.

> If you are building on Windows, see the mingw.txt file.

### Compile Time Prerequisites

You can use:
```bash
apt install -y autoconf gperf make gcc g++ bison flex
```

You need the following software to compile Icarus Verilog from source
on a UNIX-like system:

- GNU Make
  The Makefiles use some GNU extensions, so a basic POSIX
  make will not work. Linux systems typically come with a
  satisfactory make. BSD based systems (i.e., NetBSD, FreeBSD)
  typically have GNU make as the gmake program.

- ISO C++ Compiler
  The ivl and ivlpp programs are written in C++ and make use
  of templates and some of the standard C++ library. egcs and
  recent gcc compilers with the associated libstdc++ are known
  to work. MSVC++ 5 and 6 are known to definitely *not* work.

- bison and flex
  OSX note: bison 2.3 shipped with MacOS including Catalina generates
  broken code, but bison 3+ works. We recommend using the Fink
  project version of bison and flex (finkproject.org), brew version
  works fine either.

- gperf 3.0 or later
  The lexical analyzer doesn't recognize keywords directly,
  but instead matches symbols and looks them up in a hash
  table in order to get the proper lexical code. The gperf
  program generates the lookup table.

  A version problem with this program is the most common cause
  of difficulty. See the Icarus Verilog FAQ.

- readline 4.2 or later
  On Linux systems, this usually means the readline-devel
  rpm. In any case, it is the development headers of readline
  that are needed.

- termcap
  The readline library, in turn, uses termcap.

	> If you are building from git, you will also need software to generate the configure scripts.

- autoconf 2.53 or later
  This generates configure scripts from configure.ac. The 2.53
  or later versions are known to work, autoconf 2.13 is
  reported to *not* work.

### Compilation

<details>
<summary><h4><a href="https://github.com/steveicarus/iverilog/releases">Compiling From Release</a></h4></summary>

Unpack the tar-ball, `cd` into the `verilog-#########` directory,
and compile the source with the commands:

```bash
  ./configure
  make
```
</details>

<details>
<summary><h4>Compiling From GitHub</h4></summary>

If you are building from git, you have to run the command below before
compiling the source. This will generate the "configure" file, which is
automatically done when building from tarball.

```bash
  sh autoconf.sh
  ./configure
  make
```

Normally, this command automatically figures out everything it needs
to know. It generally works pretty well. There are a few flags to the
configure script that modify its behaviour:

```
	--prefix=<root>
		The default is /usr/local, which causes the tool suite to
		be compiled for install in /usr/local/bin,
		/usr/local/share/ivl, etc.

		I recommend that if you are configuring for precompiled
		binaries, use --prefix=/usr.  On Solaris systems, it is
		common to use --prefix=/opt.  You can configure for a non-root
		install with --prefix=$HOME.

	--enable-suffix
	--enable-suffix=<your-suffix>
	--disable-suffix
		Enable/disable changing the names of install files to use
		a suffix string so that this version or install can co-
		exist with other versions. This renames the installed
		commands (iverilog, iverilog-vpi, vvp) and the installed
		library files and include directory so that installations
		with the same prefix but different suffix are guaranteed
		to not interfere with each other.

	--host=<host-type>
		Compile iverilog for a different platform. You can use:
				x64_64-w64-mingw32 for building 64-bit Windows executables
				i686-w64-mingw32 for building 32-bit Windows executables
			Both options require installing the required mingw-w64 packages.
```
</details>

### (Optional) Testing

To run a simple test before installation, execute

```bash
  make check
```

The commands printed by this run might help you in running Icarus
Verilog on your own Verilog sources before the package is installed
by root.

### Installation

Now install the files in an appropriate place. (The makefiles by
default install in /usr/local unless you specify a different prefix
with the `--prefix=<path>` flag to the configure command.) You may need
to do this as root to gain access to installation directories.

```bash
	make install
```

### Uninstallation

The generated Makefiles also include the uninstall target. This should
remove all the files that `make install` creates.

## How Icarus Verilog Works

This tool includes a parser which reads in Verilog (plus extensions)
and generates an internal netlist. The netlist is passed to various
processing steps that transform the design to more optimal/practical
forms, then is passed to a code generator for final output. The
processing steps and the code generator are selected by command line
switches.

### Preprocessing

There is a separate program, `ivlpp`, that does the preprocessing. This
program implements the `` `include `` and  `` `define `` directives producing
output that is equivalent but without the directives. The output is a
single file with line number directives, so that the actual compiler
only sees a single input file. See `ivlpp/ivlpp.txt` for details.

### Parse

The Verilog compiler starts by parsing the Verilog source file. The
output of the parse is a list of Module objects in "pform". The pform
(see `pform.h`) is mostly a direct reflection of the compilation
step. There may be dangling references, and it is not yet clear which
module is the root.

One can see a human-readable version of the final pform by using the
`-P <path>` flag to the `ivl` subcommand. This will cause ivl
to dump the pform into the file named `<path>`. (Note that this is not
normally done, unless debugging the `ivl` subcommand.)

### Elaboration

This phase takes the pform and generates a netlist. The driver selects
(by user request or lucky guess) the root module to elaborate,
resolves references and expands the instantiations to form the design
netlist. (See netlist.txt.) Final semantic checks are performed during
elaboration, and some simple optimizations are performed. The netlist
includes all the behavioural descriptions, as well as gates and wires.

The `elaborate()` function performs the elaboration.

One can see a human-readable version of the final, elaborated and
optimized netlist by using the `-N <path>` flag to the compiler. If
elaboration succeeds, the final netlist (i.e., after optimizations but
before code generation) will be dumped into the file named `<path>`.

Elaboration is performed in two steps: scopes and parameters
first, followed by the structural and behavioural elaboration.

#### Scope Elaboration

This pass scans through the pform looking for scopes and parameters. A
tree of NetScope objects is built up and placed in the Design object,
with the root module represented by the root NetScope object. The
`elab_scope.cc` file contains most of the code for handling this phase.

The tail of the elaborate_scope behaviour (after the pform is
traversed) includes a scan of the NetScope tree to locate defparam
assignments that were collected during scope elaboration. This is when
the defparam overrides are applied to the parameters.

#### Netlist Elaboration

After the scopes and parameters are generated and the NetScope tree
fully formed, the elaboration runs through the pform again, this time
generating the structural and behavioural netlist. Parameters are
elaborated and evaluated by now so all the constants of code
generation are now known locally, so the netlist can be generated by
simply passing through the pform.

### Optimization

This is a collection of processing steps that perform
optimizations that do not depend on the target technology. Examples of
some useful transformations are

- eliminate null effect circuitry
- combinational reduction
- constant propagation

The actual functions performed are specified on the `ivl` command line by
the `-F` flags (see below).

### Code Generation

This step takes the design netlist and uses it to drive the code
generator (see target.h). This may require transforming the
design to suit the technology.

The `emit()` method of the Design class performs this step. It runs
through the design elements, calling target functions as the need arises
to generate actual output.

The user selects the target code generator with the `-t` flag on the
command line.

### Attributes

> NOTE: The $attribute syntax will soon be deprecated in favour of the Verilog-2001 attribute syntax, which is cleaner and standardized.

The parser accepts, as an extension to Verilog, the $attribute module
item. The syntax of the $attribute item is:

```
	$attribute (<identifier>, <key>, <value>);
```

The $attribute keyword looks like a system task invocation. The
difference here is that the parameters are more restricted than those
of a system task. The `<identifier>` must be an identifier. This will be
the item to get an attribute. The `<key>` and `<value>` are strings, not
expressions, that give the key and the value of the attribute to be
attached to the identified object.

Attributes are `[<key> <value>]` pairs and are used to communicate with
the various processing steps. See the documentation for the processing
step for a list of the pertinent attributes.

Attributes can also be applied to gate types. When this is done, the
attribute is given to every instantiation of the primitive. The syntax
for the attribute statement is the same, except that the `<identifier>`
names a primitive earlier in the compilation unit and the statement is
placed in the global scope, instead of within a module. The semicolon is
not part of a type attribute.

Note that attributes are also occasionally used for communication
between processing steps. Processing steps that are aware of others
may place attributes on netlist objects to communicate information to
later steps.

Icarus Verilog also accepts the Verilog 2001 syntax for
attributes. They have the same general meaning as with the $attribute
syntax, but they are attached to objects by position instead of by
name. Also, the key is a Verilog identifier instead of a string.

## Running `iverilog`

The preferred way to invoke the compiler is with the `iverilog`(1)
command. This program invokes the preprocessor (`ivlpp`) and the
compiler (`ivl`) with the proper command line options to get the job
done in a friendly way. See the `iverilog`(1) man page for usage details.


### EXAMPLE: Hello World

Example: Compiling `"hello.vl"`

```verilog
// ------------------------ hello.vl ----------------------------

module main();

initial
  begin
    $display("Hello World");
    $finish ;
  end

endmodule

// --------------------------------------------------------------
```

Ensure that `iverilog` is on your search path, and the vpi library
is available.

To compile the program:

```bash
	iverilog hello.vl
```

(The above presumes that `/usr/local/include` and `/usr/local/lib` are
part of the compiler search path, which is usually the case for `gcc`.)

To run the generated program:

```bash
	./a.out
```

You can use the `-o` switch to name the output command to be generated
by the compiler. See the `iverilog`(1) man page.

## Unsupported Constructs

Icarus Verilog is in development - as such it still only supports a
(growing) subset of Verilog.  Below is a description of some of the
currently unsupported Verilog features. This list is not exhaustive
and does not account for errors in the compiler. See the Icarus
Verilog web page for the current state of support for Verilog, and in
particular, browse the bug report database for reported unsupported
constructs.

  - System functions are supported, but the return value is a little
    tricky. See SYSTEM FUNCTION TABLE FILES in the iverilog man page.

  - Specify blocks are parsed but ignored in general.

  - `trireg` is not supported. `tri0` and `tri1` are supported.

  - tran primitives, i.e. `tran`, `tranif1`, `tranif0`, `rtran`, `rtranif1`,
    and `rtranif0` are not supported.

  - Net delays, of the form `wire #N foo;` do not work. Delays in
    every other context do work properly, including the V2001 form
    `wire #5 foo = bar;`

  - Event controls inside non-blocking assignments are not supported.
    i.e.: `a <= @(posedge clk) b;`

  - Macro arguments are not supported. `` `define `` macros are supported,
    but they cannot take arguments.

## Nonstandard Constructs or Behaviors

Icarus Verilog includes some features that are not part of the
IEEE1364 standard, but have well-defined meaning, and also sometimes
gives nonstandard (but extended) meanings to some features of the
language that are defined. See the "extensions.txt" documentation for
more details.

* `$is_signed(<expr>)`

	This system function returns 1 if the expression contained is
	signed, or 0 otherwise. This is mostly of use for compiler
	regression tests.

* `$sizeof(<expr>)`, `$bits(<expr>)`

	The `$bits` system function returns the size in bits of the
	expression that is its argument. The result of this
	function is undefined if the argument doesn't have a
	self-determined size.

	The `$sizeof` function is deprecated in favour of `$bits`, which is
	the same thing, but included in the SystemVerilog definition.

* `$simtime`

  The `$simtime` system function returns as a 64bit value the
	simulation time, unscaled by the time units of local
	scope. This is different from the $time and $stime functions
	which return the scaled times. This function is added for
	regression testing of the compiler and run time, but can be
	used by applications who really want the simulation time.

	Note that the simulation time can be confusing if there are
	lots of different `` `timescales`` within a design. It is not in
	general possible to predict what the simulation precision will
	turn out to be.

* `$mti_random()`, `$mti_dist_uniform`

	These functions are similar to the IEEE1364 standard $random
	functions, but they use the Mersenne Twister (MT19937)
	algorithm. This is considered an excellent random number
	generator, but does not generate the same sequence as the
	standardized $random.

### Builtin system functions

Certain of the system functions have well-defined meanings, so
can theoretically be evaluated at compile-time, instead of
using runtime VPI code. Doing so means that VPI cannot
override the definitions of functions handled in this
manner. On the other hand, this makes them synthesizable, and
also allows for more aggressive constant propagation. The
functions handled in this manner are:

* `$bits`
* `$signed`
* `$sizeof`
* `$unsigned`

Implementations of these system functions in VPI modules will be ignored.

### Preprocessing Library Modules

Icarus Verilog does preprocess modules that are loaded from
libraries via the -y mechanism. However, the only macros
defined during the compilation of that file are those that it
defines itself (or includes) or that are defined in the
command line or command file.

Specifically, macros defined in the non-library source files
are not remembered when the library module is loaded. This is
intentional. If it were otherwise, then compilation results
might vary depending on the order that libraries are loaded,
and that is too unpredictable.

It is said that some commercial compilers do allow macro
definitions to span library modules. That's just plain weird.

### Width in `%t` Time Formats

Standard Verilog does not allow width fields in the %t formats
of display strings. For example, this is illegal:

```
	$display("Time is %0t", $time);
```

Standard Verilog instead relies on the $timeformat to
completely specify the format.

Icarus Verilog allows the programmer to specify the field
width. The `%t` format in Icarus Verilog works exactly as it
does in standard Verilog. However, if the programmer chooses
to specify a minimum width (i.e., `%5t`), then for that display
Icarus Verilog will override the `$timeformat` minimum width and
use the explicit minimum width.

### vpiScope Iterator on vpiScope Objects

In the VPI, the normal way to iterate over vpiScope objects
contained within a vpiScope object, is the vpiInternalScope
iterator. Icarus Verilog adds support for the vpiScope
iterator of a vpiScope object, that iterates over *everything*
the is contained in the current scope. This is useful in cases
where one wants to iterate over all the objects in a scope
without iterating over all the contained types explicitly.

### Time 0 Race Resolution

Combinational logic is routinely modelled using always
blocks. However, this can lead to race conditions if the
inputs to the combinational block are initialized in initial
statements. Icarus Verilog slightly modifies time 0 scheduling
by arranging for always statements with ANYEDGE sensitivity
lists to be scheduled before any other threads. This causes
combinational always blocks to be triggered when the values in
the sensitivity list are initialized by initial threads.

### Nets with Types

Icarus Verilog supports an extended syntax that allows nets
and regs to be explicitly typed. The currently supported types
are logic, bool and real. This implies that `logic` and `bool`
are new keywords. Typical syntax is:

```verilog
	wire real foo = 1.0;
	reg logic bar, bat;
```
... and so forth. The syntax can be turned off by using the
`-g2` flag to iverilog, and turned on explicitly with the `-g2x`
flag to iverilog.


## Credits

Except where otherwise noted, Icarus Verilog, ivl, and ivlpp are
Copyright Stephen Williams. The proper notices are in the head of each
file. However, I have early on received aid in the form of fixes,
Verilog guidance, and especially testing from many people. Testers, in
particular, include a larger community of people interested in a GPL
Verilog for Linux.
