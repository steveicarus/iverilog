
THE ICARUS VERILOG COMPILATION SYSTEM

This tool includes a parser that parses Verilog (plus extensions) and
generates an internal netlist. The netlist is passed to various
processing steps that transform the design to more optimal/practical
forms, then passed to a code generator for final output. The
processing steps and the code generator are selected by command line
switches.

INVOKING

The vl command is the compiler driver, that invokes the parser,
optimization functions and the code generator.

Usage: vl [-s <module>] [-o <file>] [-D] [-F <name>] [-t <name>] file
       vl -h

-F <name>
	Use this flag to request an optimization function be applied
	to the netlist before it is sent to the target output
	stage. Any number of -F options may be given, to specify a
	variety of processing steps. The steps will be applied in
	order, with the output of one uses as the input to the next.

	The function is specified by name. Use the "vl -h" command to
	get a list of configured function names.

-f <assign>
	Use this flag to set a parameter value. The format of the
	assignment is <key>=<value> where key is any string up to the
	first '=', and <value> is the rest of the option. If the '='
	is omitted, then the key is assigned the empty string.

	The useful keys are defined by the functions and the target in
	use. These assignments are specifically useful for passing
	target specific information to the target backend, or
	options/parameters to optimization functions, if any are defined.

-o <file>
	Normally, the generated result is sent to standard
	output. Use the -o flag to specify an output file for the
	generated result.

-s <module>
	Normally, vl will elaborate the only module in the source
	file. If there are multiple modules, use this option to select
	the module to be used as the top-level module.

-t <name>
	Select the output format for the compiled result. Use the
	"vl -h" command to get a list of configured targets.

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

HOW IT WORKS -- STAGES OF PROCESSING

* Parse

The verilog compiler starts by parsing the verilog source file. The
output of the parse in a list of Module objects in PFORM. The pform
(see pform.h) is mostly a direct reflection of the compilation
unit. There may be dangling references, and it is not yet clear which
module is the root.

* Elaboration

This phase takes the pform and generates a netlist. The driver selects
(by user request or lucky guess) the root module to elaborate,
resolves references and expands the instantiations to form the design
netlist.

The elaborate() function performs the elaboration.

* Optimization

This is actually a collection of processing steps that perform
optimizations that do not depend on the target technology. Examples of
some useful transformations would be,

	- eliminate null effect circuitry,
	- combinational reduction
	- Constant propogation

The actual functions performed are specified on the command line by
the -F flags.

* Code Generation

This step takes the design netlist and uses it to drive the code
generator. (See target.h.) This may require transforming the
design to suit the technology.

The emit() method of the Design class performs this step. It runs
through the design elements, calling target functions as need arises
to generate actual output.

The target code generator to used is given by the -t flag on the
command line.
