
The Icarus Verilog Targets
==========================

Icarus Verilog elaborates the design, then sends to the design to code
generates (targets) for processing. New code generators can be added by
external packages, but these are the code generators that are bundled with
Icarus Verilog. The code generator is selected by the "-t" command line flag.

.. toctree::
   :maxdepth: 1

   tgt-vvp
   tgt-stub
   tgt-null
   tgt-vhdl
   tgt-vlog95
   tgt-pcb
   tgt-fpga
   tgt-pal
   tgt-sizer
   tgt-verilog
   tgt-blif
