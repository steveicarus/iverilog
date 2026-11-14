
Cadence PLI1 Modules
====================

With the cadpli module, Icarus Verilog is able to load PLI1
applications that were compiled and linked to be dynamic loaded by
Verilog-XL or NC-Verilog. This allows Icarus Verilog users to run
third-party modules that were compiled to interface with XL or
NC. Obviously, this only works on the operating system that the PLI
application was compiled to run on. For example, a Linux module can
only be loaded and run under Linux.

Icarus Verilog uses an interface module, the "cadpli" module, to
connect the worlds. This module is installed with Icarus Verilog, and
is invoked by the usual -m flag to iverilog or vvp. This module in
turn scans the extended arguments, looking for +cadpli= arguments. The
latter specify the share object and bootstrap function for running the
module. For example, to run the module product.so, that has the
bootstrap function "my_boot"::

    vvp -mcadpli a.out -cadpli=./product.so:my_boot

The "-mcadpli" argument causes vvp to load the cadpli.vpl library
module. This activates the -cadpli= argument interpreter. The
-cadpli=<module>:<boot_func> argument, then, causes vvp, through the
cadpli module, to load the loadable PLI application, invoke the
my_boot function to get a veriusertfs table, and scan that table to
register the system tasks and functions exported by that object. The
format of the -cadpli= extended argument is essentially the same as
the +loadpli1= argument to Verilog-XL.

The integration from this point is seamless. The PLI application
hardly knows that it is being invoked by Icarus Verilog instead of
Verilog-XL, so operates as it would otherwise.
