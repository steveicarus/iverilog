		THE ICARUS VERILOG COMPILATION SYSTEM 
			   September 17, 2000	


1.0 What is ICARUS Verilog?

Icarus Verilog is intended to compile ALL of the Verilog HDL as
described in the IEEE-1364 standard. Of course, it's not quite there
yet. It does currently handle a mix of structural and behavioral
constructs. For a view of the current state of Icarus Verilog, see its
home page at <http://www.icarus.com/eda/verilog>.
 
Icarus Verilog is not aimed at being a simulator in the traditional
sense, but a compiler that generates code employed by back-end
tools. These back-end tools currently include a simulator written in
C++ called VVM, another faster simulator called VVP and an XNF (Xilinx
Netlist Format) generator. See "vvm.txt" and "xnf.txt" for further
details on these back-end processors. In the future, backends are
expected for EDIF/LPM, structural Verilog, VHDL, etc.

    For instructions on how to run Icarus Verilog, see the ``iverilog''
    man page.


2.0 Building/Installing Icarus Verilog From Source

If you are starting from source, the build process is designed to be
as simple as practical. Someone basically familiar with the target
system and C/C++ compilation should be able to build the source
distribution with little effort. Some actual programming skills are
not required, but helpful in case of problems.

2.1 Compile Time Prerequisites

You need the following software to compile Icarus Verilog from source
on a UNIX-like system:

	- GNU Make
	  The Makefiles use some GNU extensions, so a basic POSIX
	  make will not work. Linux systems typically come with a
	  satisfactory make. BSD based systems (i.e. NetBSD, FreeBSD)
	  typically have GNU make as the gmake program.

	- ISO C++ Compiler
	  The ivl and ivlpp programs are written in C++ and make use
	  of templates and some of the standard C++ library. egcs and
	  recent gcc compilers with the associated libstdc++ are known
	  to work. MSVC++ 5 and 6 are known to definitely *not* work.

	- bison and flex

	- gperf 2.7
	  The lexical analyzer doesn't recognize keywords directly,
	  but instead matches symbols and looks them up in a hash
	  table in order to get the proper lexical code. The gperf
	  program generates the lookup table.

	  A version problem with this program is the most common cause
	  of difficulty. See the Icarus Verilog FAQ.

2.2 Compilation

Unpack the tar-ball and cd into the verilog-######### directory
(presumably that is how you got to this README) and compile the source
with the commands:

  ./configure
  make

Normally, this command automatically figures out every thing it needs
to know. It generally works pretty well. There are a few flags to the
configure script that modify its behavior:

	--without-ipal
	    This turns off support for Icarus PAL, whether ipal
	    libaries are installed or not.

	--prefix=<root>
	    The default is /usr/local, which causes the tool suite to
	    be compiled for install in /usr/local/bin,
	    /usr/local/share/ivl, etc.

	    I recommend that if you are configuring for precompiled
	    binaries, use --prefix=/usr. On Solaris systems, it is
	    common to use --prefix=/opt.

2.3 (Optional) Testing

To run a simple test before installation, execute

  make check

The commands printed by this run might help you in running Icarus
Verilog on your own verilog sources before the package is installed
by root.

2.4 Installation

Now install the files in an appropriate place. (The makefiles by
default install in /usr/local unless you specify a different prefix
with the --prefix=<path> flag to the configure command.) You may need
to do this as root to gain access to installation directories.

  make install

2.5 Uninstallation

The generated Makefiles also include the uninstall target. This should
remove all the files that ``make install'' creates.

3.0 How Icarus Verilog Works

This tool includes a parser which reads in Verilog (plus extensions)
and generates an internal netlist. The netlist is passed to various
processing steps that transform the design to more optimal/practical
forms, then is passed to a code generator for final output. The
processing steps and the code generator are selected by command line
switches.

3.1 Preprocessing

There is a separate program, ivlpp, that does the preprocessing. This
program implements the `include and `define directives producing
output that is equivalent but without the directives. The output is a
single file with line number directives, so that the actual compiler
only sees a single input file. See ivlpp/ivlpp.txt for details.

3.2 Parse

The verilog compiler starts by parsing the verilog source file. The
output of the parse in a list of Module objects in PFORM. The pform
(see pform.h) is mostly a direct reflection of the compilation
step. There may be dangling references, and it is not yet clear which
module is the root.

One can see a human readable version of the final PFORM by using the
``-P <path>'' flag to the compiler. This will cause iverilog to dump
the PFORM into the file named <path>.

3.3 Elaboration

This phase takes the pform and generates a netlist. The driver selects
(by user request or lucky guess) the root module to elaborate,
resolves references and expands the instantiations to form the design
netlist. (See netlist.txt.) Final semantic checks are performed during
elaboration, and some simple optimizations are performed. The netlist
includes all the behavioral descriptions, as well as gates and wires.

The elaborate() function performs the elaboration.

One can see a human readable version of the final, elaborated and
optimized netlist by using the ``-N <path>'' flag to the compiler. If
elaboration succeeds, the final netlist (i.e. after optimizations but
before code generation) will be dumped into the file named <path>.

Elaboration is actually performed it two steps: scopes and parameters
first, followed by the structural and behavioral elaboration.

3.3.1 Scope Elaboration

This pass scans through the pform looking for scopes and parameters. A
tree of NetScope objects is built up and placed in the Design object,
with the root module represented by the root NetScope object. The
elab_scope.cc and elab_pexpr.cc files contain most of the code for
handling this phase.

The tail of the elaborate_scope behavior (after the pform is
traversed) includes a scan of the NetScope tree to locate defparam
assignments that were collected during scope elaboration. This is when
the defparam overrides are applied to the parameters.

3.3.2 Netlist Elaboration

After the scopes and parameters are generated and the NetScope tree
fully formed, the elaboration runs through the pform again, this time
generating the structural and behavioral netlist. Parameters are
elaborated and evaluated by now so all the constants of code
generation are now known locally, so the netlist can be generated by
simply passing through the pform.

3.4 Optimization

This is actually a collection of processing steps that perform
optimizations that do not depend on the target technology. Examples of
some useful transformations would be,

	- eliminate null effect circuitry,
	- combinational reduction
	- Constant propagation

The actual functions performed are specified on the ivl command line by
the -F flags (See below).

3.5 Code Generation

This step takes the design netlist and uses it to drive the code
generator. (See target.h.) This may require transforming the
design to suit the technology.

The emit() method of the Design class performs this step. It runs
through the design elements, calling target functions as need arises
to generate actual output.

The user selects the target code generator with the -t flag on the
command line.

3.6 ATTRIBUTES

The parser accepts as an extension to Verilog the $attribute module
item. The syntax of the $attribute item is:

	$attribute (<identifier>, <key>, <value>);

The $attribute keyword looks like a system task invocation. The
difference here is that the parameters are more restricted then those
of a system task. The <identifier> must be an identifier. This will be
the item to get an attribute. The <key> and <value> are strings, not
expressions, that give the key and the value of the attribute to be
attached to the identified object.

Attributes are [<key> <value>] pairs and are used to communicate with
the various processing steps. See the documentation for the processing
step for a list of the pertinent attributes.

Attributes can also be applied to gate types. When this is done, the
attribute is given to every instantiation of the primitive. The syntax
for the attribute statement is the same, except that the <identifier>
names a primitive earlier in the compilation unit and the statement is
placed in global scope, instead of within a module. The semicolon is
not part of a type attribute.

Note that attributes are also occasionally used for communication
between processing steps. Processing steps that are aware of others
may place attributes on netlist objects to communicate information to
later steps.

4.0 Running iverilog

The preferred way to invoke the compiler is with the iverilog(1)
command. This program invokes the preprocessor (ivlpp) and the
compiler (ivl) with the proper command line options to get the job
done in a friendly way. See the iverilog(1) man page for usage details.

4.1 Running IVL Directly (not recommended)

    NOTE: The preferred method of running Icarus Verilog is the
    iverilog command described above. The instructions below may
    change at any time without notice.

The ivl command is the compiler driver, that invokes the parser,
optimization functions and the code generator, but not the preprocessor.

Usage: ivl <options>... file
       ivl -h
       ivl -V

-F <name>
	Use this flag to request an optimization function be applied
	to the netlist before it is sent to the target output
	stage. Any number of -F options may be given, to specify a
	variety of processing steps. The steps will be applied in
	order, with the output of one uses as the input to the next.

	The function is specified by name. Use the "ivl -h" command to
	get a list of configured function names.

-h
	Print usage information, and exit.

-m <module>
	Cause a named VPI module to be included in the module
	list. This parameter appends the named module to the end of
	the VPI_MODULE_LIST. This is an ordered list of modules to be
	loaded into the simulation at runtime.

	This list can also be set with -fVPI_MODULE_LIST=<list> which
	sets the list completely. Then, -m after this will append
	module names to the list sp specified. The default list
	includes "system".

-N <file>
	Dump the elaborated netlist to the named file. The netlist is
	the folly elaborated netlist, after all the function modules
	are applied and right before the output generator is
	called. This is an aid for debugging the compiler, and the
	output generator in particular.

-o <file>
	Normally, the generated result is sent to standard
	output. Use the -o flag to specify an output file for the
	generated result.

-P <file>
	Write the PForm of the parsed input to the specified file.
	The pform is the compiler's understanding of the input after
	parsing and before elaboration. This is an aid for debugging
	the compiler.

-p <assign>
	Use this flag to set a parameter value. The format of the
	assignment is <key>=<value> where key is any string up to the
	first '=', and <value> is the rest of the option. If the '='
	is omitted, then the key is assigned the empty string.

	The useful keys are defined by the functions and the target in
	use. These assignments are specifically useful for passing
	target specific information to the target back-end, or
	options/parameters to optimization functions, if any are defined.

-s <module>
	Normally, ivl will elaborate the only module in the source
	file. If there are multiple modules, use this option to select
	the module to be used as the top-level module.

-T [min|typ|max]
	Normally, ivl will select typ values from min:type:max
	expressions and print a warning. This flag tells the compiler
	exactly which value to choose, and suppresses the warning.

-t <name>
	Select the output format for the compiled result. Use the
	"ivl -h" command to get a list of configured targets.

-v	Print progress indications, and (if supported by the system)
	executions times in ivl precessing steps.

-V
	Print version and copyright information for ivl, and exit.

4.2 EXAMPLES

Example: Compiling "hello.vl"

------------------------ hello.vl ----------------------------
module main();
 
initial 
  begin
    $display("Hi there");
    $finish ;
  end

endmodule

--------------------------------------------------------------

Insure that "iverilog" is on your search path, and the vpi library 
is available.

To compile the program:

  iverilog hello.vl

(The above presumes that /usr/local/include and /usr/local/lib are
part of the compiler search path, which is usually the case for gcc.)

To run the program:

  ./a.out

You can use the "-o" switch to name the output command to be generated
by the compiler. See the iverilog(1) man page.

5.0 Unsupported Constructs

Icarus Verilog is in development - as such it still only supports a
(growing) subset of Verilog.  Below is a description of some of the
currently unsupported verilog features. This list is not exhaustive,
and does not account for errors in the compiler. See the Icarus
Verilog web page for the current state of support for Verilog, and in
particular, browse the bug report database for reported unsupported
constructs.

  - block disable not supported, i.e.:

            begin : foo
	        [...]
		disable foo; // sorry
		[...]
	    end

  - Functions in structural contexts are not supported.

            assign foo = user_function(a,b); // sorry
	    always @(a or b) foo = user_function(a,b); // OK

  - real data types not supported. This includes real and
    realtime. However, floating point constants in delay expressions
    are supported so that `timescale works properly.

  - System functions are supported, but the compiler presumes that
    they return 32 bits. This is the typical case.

  - Specify blocks are parsed but ignored in general.

  - trireg is not supported. tri0 and tri1 are supported.


6.0 CREDITS

Except where otherwise noted, Icarus Verilog, ivl and ivlpp are
Copyright Stephen Williams. The proper notices are in the head of each
file. However, I have early on received aid in the form of fixes,
Verilog guidance, and especially testing from many people, including
(in alphabetical order):

	Eric Aardoom <eric_aardoom@yahoo.com>
	Stephan I. Boettcher <stephan@nevis.columbia.edu>
	Ed Carter <r47652@email.sps.mot.com>
	Larry Doolittle <LRDoolittle@lbl.gov>
	Guy Hutchison <ghutchis@pacbell.net>
	Ales Hvezda <ahvezda@seul.org>
	Venkat Iyer <venkat@comit.com>
	Yasuhisa Kato <ykato@mac.com>
        James Lee <jml@jmlzone.com>
	Peter Monta <pmonta@halibut.imedia.com>
	Daniel H. Nelsen <dhn@qedinc.com>
	Stefan Petersen <spe@geda.seul.org>
	Jason Schonberg <schonm@yahoo.com>
	Stuart Sutherland <stuart@sutherland.com>
	Stephen Tell <tell@cs.unc.edu>
        Stefan Theide <Stefan.Thiede@sv.sc.philips.com>
	Steve Wilson <stevew@home.com>

and others. Testers in particular include a larger community of people
interested in a GPL Verilog for Linux. Special thanks to Steve Wilson
for collecting and organizing the test suite code for all those testers.


6.1 PORT MAINTAINERS

This is a list of people who have created ports and precompiled
packages for various operating systems. These folks have graciously
taken on the task of building Icarus Verilog on their systems and
bundled it into neat packages.(+) If you want to be added to the list (or
removed from the list) send e-mail to me.

      FreeBSD/{Intel,alpha}
	Ying-Chieh Liao <ijliao@FreeBSD.org>

      Linux/{alpha,Intel} (RPMS)
	Stephen Williams <steve@icarus.com>

      Linux/* (.debs)
	Hamish Moffatt <hamish@rising.com.au>

      Macintosh -- MacO/S
	Yasuhisa Kato <ykato@mac.com>

      NetBSD/*
	Dan McMahill <mcmahill@mtl.mit.edu>

      Solaris/SPARC packages (.pkg)
	Dan McMahill <mcmahill@mtl.mit.edu>

      Cygwin32/*
        Venkat Iyer <venkat@comit.com>

      Mingw32
        Venkat Iyer <venkat@comit.com>

(+) These are not the only systems where Icarus Verilog has been run,
just the systems where precompiled binaries are publicly available.


6.2 TEST SUITE MANAGER

Steve Wilson <stevew@home.com> or <stevew@intrinsix.com> has taken on
the large task of managing the test suite. He has maintained the
regression test scripts, the driver list, received submissions from
myself and others, and has written a great many tests on his own. Any
compiler writer, for any language, will tell you that the test suite
is at least as important as the compiler code itself.

