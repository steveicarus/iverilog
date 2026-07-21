# Welcome to *IVL_UVM* project

It is a humble attempt to implement some of the basic UVM features on open-source Icarus Verilog simulator and eventually port it to Verilator as well. 

Given the light-weight support of SystemVerilog by Icarus as of Dec 2020, IVL_UVM code base is also very rudimentary compared to Accellera UVM implementation. We hope to continue enhancing it as and when the Icarus SV support improves.

It is very important to note that current support is *very, very limited* and is just a start. We have staretd with:

* Basic Messaging support
* Command Line Processor features
* UVM Test feature

Feel free to try it out and send constructive comments via Discussion forum/Issues. Any inappropriate comments shall be deleted without any notice whatsoever. 

As of now, IVL_UVM runs on latest/development build of Icarus (and NOT the stable v11.0 branch). Should you need the v11.0 support, feel free to raise a request, we will see if that is doable.

## Setup
Please ensure to setup IVL_UVM path via the setup.csh, in CSH/TCSH do:
* source setup.csh

Many scripts/Makefiles refer to an env-variable that's set via this setup file. 

To run basic tests, do:

* cd ivl_uvm_regress/run_dir
* make vw0
* make vw1
* make vw2

Thanks for trying this IVL_UVM code base.

## ABV support via OVL
Assertion Based Verificaiton (ABV) is a powerful technique used by both RTL designers and Verification engineers. Accellera OVL (Open Verificaiton Library) was developed to provide a set of ready-to-use library elements with assertions for common elements/behaviors. We now comppiled OVL with Icarus and did some small clean-up in this release. Further TODOs are below:

* Integrate UVM reporting
* Tighter controf of MAX_QUIT_COUNT in more-UVM-like manner (than compile time in old OVL)
* Better debug

To run simple OVL demo, do:

* cd ivl_uvm_tests/ivl_uvm_ovl_demo/ivl_uvm_ovl_alw
* make 

Cheers
