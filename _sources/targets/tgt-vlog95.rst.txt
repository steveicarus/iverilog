
The Verilog '95 Code Generator (-tvlog95)
=========================================

Icarus Verilog contains a code generator to emit 1995 compliant Verilog from
the input Verilog netlist. This allows Icarus Verilog to function as a Verilog
> 1995 to Verilog 1995 translator. The main goal of the project was to convert
@*, ANSI style arguments and other constructs to something allowed in 1995
Verilog.

Invocation
----------

To translate a Verilog program to 1995 compliant Verilog, invoke "iverilog"
with the -tvlog95 flag::

  % iverilog -tvlog95 -o my_design_95.v my_design.v

The generated Verilog will be placed in a single file (a.out by default), even
if the input Verilog is spread over multiple files.

Generator Flags
---------------

* -pspacing=N

  Set the indent spacing (the default is 2).

* -pallowsigned=1

  Allow emitting the various signed constructs as an extension to 1995 Verilog
  (off by default).

* -pfileline=1

  Emit the original file and line information as a comment for each generated
  line (off by default).

Structures that cannot be converted to 1995 compatible Verilog
--------------------------------------------------------------

The following Verilog constructs are not translatable to 1995 compatible Verilog:

* Automatic tasks or functions.

* The power operator (**). Expressions of the form (2**N)**<variable> (where N
  is a constant) can be converter to a shift.

* Some System Verilog constructs (e.g. final blocks, ++/-- operators,
  etc.). 2-state variables are converted to 4-state variables.

Icarus extensions that cannot be translated:

* Integer constants greater than 32 bits.

* Real valued nets.

* Real modulus.

* Most Verilog-A constructs.


Known Issues and Limitations
----------------------------

Some things are just not finished and should generate an appropriate
warning. Here is a list of the major things that still need to be looked at.

* There are still a few module instantiation port issues (pr1723367 and
  partselsynth).

* inout ports are not converted (tran-VP).

* Variable selects of a non-zero based vector in a continuous assignment are
  not converted.

* There is no support for translating a zero repeat in a continuous
  assignment. It is currently just dropped.

* A pull device connected to a signal select is not translated correctly (this
  may be fixed).

* L-value indexed part selects with a constant undefined base in a continuous
  assignment are not translated.

* Logic gates are not arrayed exactly the same as the input and the instance
  name is not always the same.

* The signed support does not generate $signed() or $unsigned() function calls
  in a continuous assignment expression.

* The special power operator cases are not converted in a continuous
  assignment.

* Currently a signed constant that sets the MSB in an unsigned context will be
  displayed as a negative value (e.g. bit = 1 translates to bit = -1).

* Can net arrays, etc. be unrolled?

* Can generate blocks be converted?

