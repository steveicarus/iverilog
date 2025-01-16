
The VHDL Code Generator (-tvhdl)
================================

Icarus Verilog contains a code generator to emit VHDL from the Verilog
netlist. This allows Icarus Verilog to function as a Verilog to VHDL
translator.

Invocation
----------

To translate a Verilog program to VHDL, invoke "iverilog" with the -tvhdl
flag::

  % iverilog -t vhdl -o my_design.vhd my_design.v

The generated VHDL will be placed in a single file (a.out by default), even if
the Verilog is spread over multiple files.

Flags
-----

* -pdebug=1

  Print progress messages as the code generator visits each part of the
  design.

* -pdepth=N

  Only output VHDL entities for modules found at depth < N in the
  hierarchy. N=0, the default, outputs all entities. For example, -pdepth=1
  outputs only the top-level entity.

Supported Constructs
--------------------

TODO

Limitations
-----------

Signal Values and Resolution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are several cases where the behaviour of the translated VHDL deviates
from the source Verilog:

* The result of division by zero is x in Verilog but raises an exception in
  VHDL.

* Similarly, the result of reading past the end of an array in Verilog is x,
  whereas VHDL raises an exception.

* Any signal that is driven by two or more processes will have the value
  'U'. This is the result of the signal resolution function in the
  std_logic_1164 package.

Constructs Not Supported
^^^^^^^^^^^^^^^^^^^^^^^^

The following Verilog constructs cannot be translated to VHDL:

* fork and join

* force and release

* disable

* real-valued variables

* switches

* hierarchical dereferencing

Other Limitations
^^^^^^^^^^^^^^^^^

* The test expressions in case statements must be constant.

* Translation of a parameter to a corresponding VHDL generic
  declaration. Instead the default parameter value is used.

