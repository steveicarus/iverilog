		THE ICARUS VERILOG COMPILATION SYSTEM 
			     May 9, 1999		


1.0 What is ICARUS Verilog(IVL)?

Icarus Verilog is intended to compile ALL of the Verilog HDL as described
in the IEEE-1364 standard. Of course, it's not quite there yet. It does
currently handle a mix of structural and behavioral constructs.
 
IVL is not aimed at being a simulator in the traditional sense, but a
compiler that generates code employed by back-end tools. These back-
end tools currently include a simulator written in C++ called VVM 
and an XNF (Xilinx Netlist Format) generator. See "vvm.txt" and
"xnf.txt" for further details on the back-end processors.

2.0 How IVL Works

This tool includes a parser which reads in Verilog (plus extensions)
and generates an internal netlist. The netlist is passed to various
processing steps that transform the design to more optimal/practical
forms, then passed to a code generator for final output. The
processing steps and the code generator are selected by command line
switches.

2.1 Parse

The verilog compiler starts by parsing the verilog source file. The
output of the parse in a list of Module objects in PFORM. The pform
(see pform.h) is mostly a direct reflection of the compilation
unit. There may be dangling references, and it is not yet clear which
module is the root.

One can see a human readable version of the final PFORM by using the
``-P <path>'' flag to the compiler. This will cause ivl to dump the
PFORM into the file named <path>.

2.2 Elaboration

This phase takes the pform and generates a netlist. The driver selects
(by user request or lucky guess) the root module to elaborate,
resolves references and expands the instantiations to form the design
netlist.

The elaborate() function performs the elaboration.

One can see a human readable version of the final, elaborated and
optimized netlist by using the ``-N <path>'' flag to the compiler. If
elaboration succeeds, the final netlist (i.e. after optimizations but
before code generation) will be dumped into the file named <path>.

2.3 Optimization

This is actually a collection of processing steps that perform
optimizations that do not depend on the target technology. Examples of
some useful transformations would be,

	- eliminate null effect circuitry,
	- combinational reduction
	- Constant propagation

The actual functions performed are specified on the command line by
the -F flags (See below).

2.4 Code Generation

This step takes the design netlist and uses it to drive the code
generator. (See target.h.) This may require transforming the
design to suit the technology.

The emit() method of the Design class performs this step. It runs
through the design elements, calling target functions as need arises
to generate actual output.

The target code generator to used is given by the -t flag on the
command line.

3.0 Building/Installing IVL

Unpack the tar-ball and cd into the verilog-######### directory.

./configure
make
cd vvm
make

Now install the files in an appropriate place. (The makefiles by
default install in /usr/local unless you specify a different prefix
with the --prefix=<path> flag to the configure command.) Do this as
root.

make install
cd vvm
make install

4.0 Running IVL

The ivl command is the compiler driver, that invokes the parser,
optimization functions and the code generator.

Usage: ivl <options>... file
       ivl -h

-F <name>
	Use this flag to request an optimization function be applied
	to the netlist before it is sent to the target output
	stage. Any number of -F options may be given, to specify a
	variety of processing steps. The steps will be applied in
	order, with the output of one uses as the input to the next.

	The function is specified by name. Use the "ivl -h" command to
	get a list of configured function names.

-f <assign>
	Use this flag to set a parameter value. The format of the
	assignment is <key>=<value> where key is any string up to the
	first '=', and <value> is the rest of the option. If the '='
	is omitted, then the key is assigned the empty string.

	The useful keys are defined by the functions and the target in
	use. These assignments are specifically useful for passing
	target specific information to the target back-end, or
	options/parameters to optimization functions, if any are defined.

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

-s <module>
	Normally, ivl will elaborate the only module in the source
	file. If there are multiple modules, use this option to select
	the module to be used as the top-level module.

-t <name>
	Select the output format for the compiled result. Use the
	"ivl -h" command to get a list of configured targets.

ATTRIBUTES

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

Currently, type attributes are only supported for UDP types.

Note that attributes are also occasionally used for communication
between processing steps. Processing steps that are aware of others
may place attributes on netlist objects to communicate information to
later steps.

4.1 EXAMPLES

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

Insure that "ivl" is on your search path, and the library 
libvvm.a is available.

For csh - 

setenv PATH /usr/local/bin:$PATH
setenv LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

ivl -t vvm -o hello.cc hello.vl
g++ hello.cc -o hello -lvvm

(The above presumes that /usr/local/include and /usr/local/lib are
part of the compiler search path, which is usually the case for egcs.)

To run the program

./hello


5.0 Unsupported Constructs

IVL is in development - as such it still only supports a (growing) subset
of verilog.  Below is a description of some of the currently unsupported
verilog features.

  - The "?" operator. Example: count = val ? 1 : 0;

  - Ranges within parameter definitions:
    Example: parameter [15:0] seed = 16'ha3; 

    [Note: IEEE Std: 1364-1995 does not allow the syntax.]

  - The "&&" operator:
    Example: if (a && 0) do = 1;

  - The "===" operator:  Example: if( a === b) do = 1;

  - The ">=" operator: Example: if ( a >= 0) do = 1;

  - The ">" operator: Example: if ( a > 0) do = 1;
  
  - The "<=" operator: Example: if ( a <= 0) do = 1;

  - The "<<" shift operator: Example: a = 8'b0000_0010 << 1;  

  - Min/Typ/Max expressions: Example:  a = (1 : 6 : 14);

  - Inversion of a vector with a bit operator:
    Example: reg [7:0] a;   a = !(8'h01);  

  - The "!==" operator:  Example: if( a !== b) do = 1;

  - Expansion of a string into a larger variable:
    Example: reg [0:15] b;  b = "b"; 

  - Function declarations/calls.

  - Non-scalar memories, i.e. other than registers. 
    Example: reg [1:0] b [2:0]; 

  - Delay list. Example:  sample #(9,99) sample1(a,b);

  - Bit ranges within IF. Example: if (a[2:3])  do = 1;
 
  - Forever key word.

  - Repeat key word.

  - Assignment timing delay: Example: a = #1 0;   #1 a = #2 ~a;

  - Bit Ranges within $write, $display.

  - `timescale directive

  - Task declarations/calls.

