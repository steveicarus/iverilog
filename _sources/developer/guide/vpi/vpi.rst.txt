
VPI Modules in Icarus Verilog
================================

The VPI interface for Icarus Verilog works by creating from a
collection of PLI applications a single vpi module. The vpi module
includes compiled code for the applications linked together (with any
other libraries that the applications need) into a module with two
exported symbols, the vpip_set_callback function and the
vlog_startup_routines array.

The product that wishes to invoke the module (normally at run time) loads
the module, locates and calls the vpip_set_callback function to pass the
the module a jump table that allows the module to access the VPI routines
implemented by the product, then locates the vlog_startup_routines table
and calls all the startup routines contained in that table. It is possible
for a product to link with many modules. In that case, all the modules are
linked in and startup routines are called in order.

The product that uses vpi modules uses the environment variable
VPI_MODULE_PATH as a ':' separated list of directories. This is the
module search path. When a module is specified by name (using whatever
means the product supports) the module search path is scanned until
the module is located.

The special module names "system.vpi", "v2005_math.vpi", "v2009.vpi",
and "va_math.vpi" are part of the core Icarus Verilog distribution and
include implementations of the standard system tasks/functions. The
additional special module names "vhdl_sys.vpi" and "vhdl_textio.vpi"
include implementations of private functions used to support VHDL.

Compiling A VPI Module
----------------------

See the documentation under: :doc:`Using VPI <../../../usage/vpi>`

Tracing VPI Use
---------------

The vvp command includes the ability to trace VPI calls. This is
useful if you are trying to debug a problem with your code. To
activate tracing simply set the VPI_TRACE environment variable, with
the path to a file where trace text gets written. For example::

	setenv VPI_TRACE /tmp/foo.txt

This tracing is pretty verbose, so you don't want to run like this
normally. Also, the format of the tracing messages will change
according to my needs (and whim) so don't expect to be able to parse
it in software.
