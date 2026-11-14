
Verilog Attributes
==================

This is a description of the various attributes that the Icarus Verilog tools
understand. The attributes are attached to objects using the "(\* ... \*)"
syntax, which is described by the Verilog LRM.

Attributes that start with "ivl\_" are Icarus Verilog specific are are probably
ignored by other tools.

Optimizations
-------------

* ivl_do_not_elide (snapshot 20140619 or later)

  This applies to signals (i.e. reg, wire, etc.) and tells the optimizer to
  not elide the signal, even if it is not referenced anywhere in the
  design. This is useful if the signal is for some reason only accessed by
  VPI/PLI code.

Synthesis
---------

* ivl_synthesis_cell

  Applied to a module definition, this tells the synthesizer that the module
  is a cell. The synthesizer does not descend into synthesis cells, as they
  are assumed to be primitives in the target technology.

* ivl_synthesis_off

  Attached to an "always" statement, this tells the synthesizer that the
  statement is not to be synthesized. This may be useful, for example, to tell
  the compiler that a stretch of code is test-bench code.
