
The Icarus Verilog Targets
==========================

Icarus Verilog elaborates the design, then sends to the design to code
generates (targets) for processing. new code generators can be added by
external packages, but these are the code generators that are bundled with
Icarus Verilog. The code generator is selected by the "-t" command line flag.

.. toctree::
   :maxdepth: 1

   vvp
   stub
   null
   vhdl
   verilog95
   pcb
