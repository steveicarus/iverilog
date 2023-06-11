
Swift Model Support (Preliminary)
=================================

    Copyright 2003 Stephen Williams

  NOTE: SWIFT support does not work yet, these are provisional
  instructions, intended to show what's supposed to happen when I get
  it working.

Icarus Verilog support for SWIFT models is based on the LMTV interface
module from Synopsys. This module is normally distributed along with
the SWIFT models proper. This module can be linked with Icarus Verilog
via the cadpli compatibility object. (See cadpli.txt.)

* Preliminaries

First, you need the LMC_HOME environment variable set to point to the
installed directory for your SWIFT software. This setup is documented
in your SWIFT model documentation.

* Compilation

When compiling your Verilog design to include a SWIFT model, you need
to include wrappers for the model you intend to use. You may choose to
use ncverilog or verilogxl compatible wrappers, they work the
same. Locate your smartmodel directory, and include it in your command
file like so::

     +libdir+.../smartmodel/sol/wrappers/verilogxl

The wrappers directory includes Verilog modules that wrap your SWIFT
module, and with this +libdir+ statement in your command file, the
Icarus Verilog compiler will be able to locate these wrappers. The
wrappers in turn invoke the $lm_model system tasks that are the LMTV
support for your model.

  NOTE: This example uses the solaris directory of VerilogXL support
  files as a source of wrappers. The files of interest, however, are
  written in Verilog and are identical for all supported platforms, so
  long as you choose the verilogxl or ncverilog files.

* Execution

After your simulation is compiled, run the simulation with the vvp
command, like this::

      % vvp -mcadpli a.out -cadpli=$LMC_HOME/lib/x86_linux.lib/swiftpli.so:swift_boot

What this command line means is::

	-mcadpli
	   Include the cadpli compatibility module

	a.out
	   This is your compiled vvp file

	-cadpli=$LMC_HOME/lib/x86_linux.lib/swiftpli.so:swift_boot
	   This tells the cadpli module to load the swiftpli.so
	   shared object, and boot it. This is code that comes with
	   your SWIFT modules, and provides the generic SWIFT
	   capabilities (lm_* system tasks) needed by the module
	   itself.

Once you start the vvp command, the SWIFT infrastructure will be
initialized as part of the simulation setup, and all should work
normally from here.

