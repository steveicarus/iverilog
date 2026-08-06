
The PCB Code Generator (-tpcb)
==============================

The PCB target code generator is designed to allow a user to enter a netlist
in Verilog format, then generate input files for the GNU PCB layout program.

Invocation
----------

The PCB target code generation is invoked with the -tpcb flag to the iverilog
command. The default output file, "a.out", contains the generated .PCB
file. Use the "-o" flag to set the output file name explicitly. The default
output file contains only the elements. To generate a "netlist" file, add the
flag "-pnetlist=<path>" command line flag.

Altogether, this example generates the foo.net and foo.pcb files from the
foo.v source file::

  % iverilog -tpcb -ofoo.pcb -pnetlist=foo.net foo.v

Flags
-----

* -o <path>

  Set the output (pcb) file path

* -pnetlist=path

  Write a netlist file to the given path.

Attributes Summary
------------------

Attributes are attached to various constructs using the Verilog "(\* \*)"
attribute syntax.

* ivl_black_box

  Attached to a module declaration or module instantiation, this indicates
  that the module is a black box. The code generator will create an element
  for black box instances.

Parameters Summary
------------------

Within modules, The PCB code generator uses certain parameters to control
details. Parameters may have defaults, and can be overridden using the usual
Verilog parameter override syntax. Parameters have preferred types.

* description (string, default="")

  The "description" is a text string that describes the black box. This string
  is written into the description field of the PCB Element.

* value (string, default="")

  The "value" is a text tring that describes some value for the black
  box. Like the description, the code generator does not interpret this value,
  other then to write it to the appropriate field in the PCB Element."
