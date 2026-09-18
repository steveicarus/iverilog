
Loadable Targets
================

Icarus Verilog supports dynamically loading code generator modules to
perform the back-end processing of the completed design. The user
specifies on the command line the module to load. The compiler loads
the module (once the design is compiled and elaborated) and calls it
to finally handle the design.

Loadable target modules implement a set of functions that the core
compiler calls to pass the design to it, and the module in turn uses a
collection of functions in the core (the API) to access details of the
design.

Loading Target Modules
----------------------

The target module loader is invoked with the ivl flag "-tdll". That
is, the DLL loader is a linked in target type. The name of the target
module to load is then specified with the DLL flag, i.e. "-fDLL=<path>".

Compiling Target Modules
------------------------

<write me>

Loadable Target Module Api
--------------------------

The target module API is defined in the ivl_target.h header file. This
declares all the type and functions that a loadable module needs to
access the design.


About Specific Expression Types
-------------------------------

In this section find notes about the various kinds of expression
nodes. The notes here are in addition to the more general
documentation in the ivl_target.h header file.

* IVL_EX_CONCAT

  The concatenation operator forms an expression node that holds the
  repeat count and all the parameter expressions. The repeat count is
  an integer that is calculated by the core compiler so it fully
  evaluated, and *not* an expression.

  The parameter expressions are retrieved by the ivl_expr_parm method,
  with the index increasing as parameters go from left to right, from
  most significant to least significant. (Note that this is different
  from the order of bits within an expression node.)

* IVL_EX_NUMBER

  This is a constant number. The width is fully known, and the bit
  values are all represented by the ASCII characters 0, 1, x or z. The
  ivl_expr_bits method returns a pointer to the least significant bit,
  and the remaining bits are ordered from least significant to most
  significant. For example, 5'b1zzx0 is the 5 character string "0xzz1".
