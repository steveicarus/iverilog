
The BLIF Code Generator (-tblif)
================================

The BLIF code generator supports emitting the design to a blif format
file as accepted by:

    ABC: A System for Sequential Synthesis and Verification
    <http://www.eecs.berkeley.edu/~alanmi/abc/>

This package contains tools sometimes used by ASIC designers. This
blif target emits .blif file that the ABC system can read int via
the "read_blif" command.


USAGE
-----

This code generator is intended to process structural Verilog source
code. To convert a design to blif, use this command::

  % iverilog -tblif -o<path>.blif  <source files>...

The source files can be Verilog, SystemVerilog, VHDL, whatever Icarus
Verilog supports, so long as it elaborates down to the limited subset
that the code generator supports. In other words, the files must be
structural.

The root module of the elaborated design becomes the model is
generated. That module may instantiate sub-modules and so on down the
design, completing the design. The output model is flattened, so it
doesn't invoke any subcircuits. Bit vectors are exploded out at the
model ports and internally. This is necessary since blif in particular
and ABC in general processes bits, not vectors.


LIMITATIONS
-----------

Currently, only explicit logic gates and continuous assignments are
supported.

The design must contain only one root module. The name of that root
module becomes the name of the blif model in the ".model" record.
