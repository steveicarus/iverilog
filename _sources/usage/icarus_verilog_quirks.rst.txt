
Icarus Verilog Quirks
=====================

This is a list of known quirks that are presented by Icarus Verilog. The idea
of this chapter is to call out ways that Icarus Verilog differs from the
standard, or from other implementations.

This is NOT AN EXHAUSTIVE LIST. If something is missing from this list, let us
know and we can add documentation.

Unsized Numeric Constants are Not Limited to 32 Bits
----------------------------------------------------

The Verilog standard allows Verilog implementations to limit the size of
unsized constants to a bit width of at least 32. That means that a constant
17179869183 (``36'h3_ffff_ffff``) may overflow some compilers. In fact, it
is common to limit these values to 32 bits. However, a compiler may just as
easily choose another width limit, for example 64 bits. That value is
equally good.

However, it is not required that an implementation truncate at 32 bits, and
in fact Icarus Verilog does not truncate at all. It will make the unsized
constant as big as it needs to be to hold the value accurately. This is
especially useful in situations like this;

.. code-block:: verilog

 reg [width-1:0] foo = 17179869183;

The programmer wants the constant to take on the width of the reg, which in
this example is parameterized. Since constant sizes cannot be parameterized,
the programmer ideally gives an unsized constant, which the compiler then
expands/contracts to match the l-value.

Also, by choosing to not ever truncate, Icarus Verilog can handle code written
for a 64 bit compiler as easily as for a 32 bit compiler. In particular, any
constants that the user does not expect to be arbitrarily truncated by their
compiler will also not be truncated by Icarus Verilog, no matter what that
other compiler chooses as a truncation point.

Unsized Expressions
-------------------

Icarus Verilog classes any expression containing an unsized numeric constant
or unsized parameter value that is not part of a self-determined operand as
an unsized expression. When calculating the bit width of an unsized expression,
it extends the width of the expression to avoid arithmetic overflow or
underflow; in other words, the expression width will be made large enough to
represent any possible arithmetic result of the expression. If the expression
contains operations that do not follow the normal rules of arithmetic (e.g. an
explicit or implicit cast between signed and unsigned values), the expression
width will be extended to at least the width of an integer.

An exception to the above is made if the expression contains a shift or power
operator with a right hand operand that is a non-constant unsized expression.
In this case any expansion of the expression width due to that operation is
limited to the width of an integer, to avoid excessive expression widths
(without this, an expression such as ``2**(i-1)``, where ``i`` is an integer,
would be expanded to 2\**33 bits).

The above behaviour is a deviation from the Verilog standard, which states
that when calculating an expression width, the width of an unsized constant
number is the same as the width of an integer. If you need strict standard
compliance (for compatibility with other EDA tools), then the compiler has
a command line option, ``-gstrict-expr-width``, which disables the special
treatment of unsized expressions. With this option, the compiler will output
a warning message if an unsized numeric constant is encountered that cannot
be represented in integer-width bits and will truncate the value.

If you are simulating synthesisable code, it is recommended that the
``-gstrict-expr-width`` option is used, as this eliminates a potential
source of synthesis vs. simulation mismatches.

Unsized Parameters
------------------

Icarus Verilog classes any parameter declaration that has no explicit or
implicit range specification as an unsized parameter declaration. When
calculating the bit width of the final value expression for the parameter,
it follows the same rules as it does for unsized expressions, regardless of
whether or not the expression contains any unsized numeric constants.

If the final value expression for an unsized parameter is an unsized
expression (i.e. does contain unsized numeric constants), any subsequent use
of that parameter will be treated as if it was an unsized numeric constant.
If not, it will be treated as if it was a numeric constant of the appropriate
size. For example, with the declarations:

.. code-block:: verilog

  localparam Value1 =  'd3 +  'd2;
  localparam Value2 = 2'd3 + 2'd2;

any subsequent use of ``Value1`` will be treated as if the programmer had
written ``'d5`` and any subsequent use of ``Value2`` will be treated as if
the programmer had written ``3'd5``. In particular, note that ``Value2`` can
be used as a concatenation operand, but ``Value1`` cannot.

The above behaviour is a deviation from the Verilog standard. As for
unsized expressions, if you need strict standard compliance. use the
``-gstrict-expr-width`` compiler option.

Unsized Expressions as Arguments to Concatenation
-------------------------------------------------

The Verilog standard clearly states in 4.1.14:

  "Unsized constant numbers shall not be allowed in  concatenations. This
  is because the size of each  operand in the concatenation is needed to
  calculate the complete size of the concatenation."

So for example the expression ``{1'b0, 16}`` is clearly illegal. It also stands
to reason that ``{1'b0, 15+1}`` is illegal, for exactly the same justification.
What is the size of the expression (15+1)? Furthermore, it is reasonable to
expect that (16) and (15+1) are exactly the same so far as the compiler is
concerned.

Unfortunately, Cadence seems to feel otherwise. In particular, it has been
reported that although ``{1'b0, 16}`` causes an error, ``{1'b0, 15+1}`` is
accepted. Further testing shows that any expression other than a simple
unsized constant is accepted there, even if all the operands of all the
operators that make up the expression are unsized integers.

This is a semantic problem. Icarus Verilog doesn't limit the size of integer
constants. This is valid as stated in 2.5.1 Note 3:

  "The number of bits that make up an unsized number (which is a simple
  decimal number or a number without the size specification) shall be
  **at least** 32." [emphasis added]

Icarus Verilog will hold any integer constant, so the size will be as large as
it needs to be, whether that is 64 bits, 128 bits, or more. With this in mind,
what is the value of these expressions?

.. code-block:: verilog

  {'h1_00_00_00_00}
  {'h1 << 32}
  {'h0_00_00_00_01 << 32}
  {'h5_00_00_00_00 + 1}

These examples show that the standard is justified in requiring that the
operands of concatenation have size. The dispute is what it takes to cause
an expression to have a size, and what that size is. Verilog-XL claims that
(16) does not have a size, but (15+1) does. The size of the expression (15+1)
is the size of the adder that is created, but how wide is the adder when
adding unsized constants?

One might note that the quote from section 4.1.14 says "Unsized constant
numbers shall not be allowed." It does not say "Unsized expressions...", so
arguably accepting (15+1) or even (16+0) as an operand to a concatenation is
not a violation of the letter of the law. However, the very next sentence of
the quote expresses the intent, and accepting (15+1) as having a more defined
size then (16) seems to be a violation of that intent.

Whatever a compiler decides the size is, the user has no way to predict it,
and the compiler should not have the right to treat (15+1) any differently
then (16). Therefore, Icarus Verilog takes the position that such expressions
are unsized and are not allowed as operands to concatenations. Icarus Verilog
will in general assume that operations on unsized numbers produce unsized
results. There are exceptions when the operator itself does define a size,
such as the comparison operators or the reduction operators. Icarus Verilog
will generate appropriate error messages.

Scope of Macro Defines Doesn't Extend into Libraries
----------------------------------------------------

Icarus Verilog does preprocess modules that are loaded from libraries via the
``-y`` mechanism to substitute macros and load includes. However, the only
macros defined during compilation of an automatically loaded library module
file are those that it defines itself (or includes) or that are defined on the
command line or in the command file. Specifically, macros defined in the non-
library source files are not remembered when the library module is loaded, and
macros defined in a library module do not escape into the rest of the design.
This is intentional. If it were otherwise, then compilation results might vary
depending on the order that libraries are loaded, and that is unacceptable.

For example, given sample library module ``a.v``:

.. code-block:: verilog

  `define MACRO_A 1
  module a(input x);
    always @(x) $display("x=",x);
  endmodule

and sample library module ``b.v``:

.. code-block:: verilog

  module b(input y);
  `ifdef MACRO_A
    always @(y) $display("MACRO_A is defined",,y);
 `else
    always @(y) $display("MACRO_A is NOT defined",,y);
  `endif
  endmodule

If a program instantiates both of these modules, there is no way to know
which will be loaded first by the compiler, so if the definition of
``MACRO_A`` in ``a.v`` were to escape, then there is no way to predict or
control whether ``MACRO_A`` is defined when ``b.v`` is processed. So the
preprocessor processes automatic library module files as if they are in
their own compilation unit, and you can know that ``MACRO_A`` will not be
defined in ``b.v`` unless it is defined on the command line (a ``-D`` flag)
or in the command file (a ``+define+`` record.)

Of course if ``a.v`` and ``b.v`` were listed in the command file or on the
command line, then the situation is different; the order is clear. The files
are processed as if they were concatenated in the order that they are listed
on the command line. The non-library modules are all together in a main
compilation unit, and they are all processed before any library modules are
loaded.

It is said that some commercial compilers do allow macro definitions to span
library modules. That's just plain weird. However, there is a special case
that Icarus Verilog does handle. Preprocessor definitions that are made in
files explicitly listed on the command line or in the command file, do pass
into implicitly loaded library files. For example, given the source file
``x.v``:

.. code-block:: verilog

  module main;
    reg foo;
    b dut(foo);
  endmodule
  `define MACRO_A

and the library module file ``b.v`` described above, the situation is well
defined, assuming the ``x.v`` file is listed on the command line or in the
command file. The library module will receive the ``MACRO_A`` definition
from the last explicitly loaded source file. The position of the define of
``MACRO_A`` in the explicitly loaded source files does not matter, as all
explicitly loaded source files are preprocessed before any library files
are loaded.

Continuous Assign L-Values Can Implicit-Define Wires
----------------------------------------------------

The IEEE 1364-2001 standard, Section 3.5, lists the cases where nets may be
implicitly created. These include:

- identifier is a module port
- identifier is passed as a port to a primitive or module

This does not seem to include continuous assignment l-values (or r-values)
so the standard does not justify allowing implicit declarations of nets by
continuous assignment.

However, it has been reported that many Verilog compilers, including the big
name tools, do allow this. So, Icarus Verilog will allow it as well, as an
extension. If ``-gxtypes`` (the default) is used, this extension is enabled.
To turn off this behavior, use the ``-gno-xtypes`` flag.

Dumping Array Words (``$dumpvars``)
-----------------------------------

Icarus has the ability to dump individual array words. They are only dumped
when explicitly passed to $dumpvars. They are not dumped by default. For
example given the following:

.. code-block:: verilog

  module top;
    reg [7:0] array [2:0];
    initial begin
      $dumpvars(0, array[0], array[1]);
      ...
    end
  endmodule

``array[0]`` and ``array[1]`` will be dumped whenever they change value. They
will be displayed as an escaped identifier and GTKWave fully supports this.
Note that this is an implicitly created escaped identifier that could conflict
with an explicitly created escaped identifier. You can automate adding the
array word by adding an index definition

.. code-block:: verilog

  integer idx;

and replacing the previous $dumpvars statement with

.. code-block:: verilog

  for (idx = 0; idx < 2; idx = idx + 1) $dumpvars(0, array[idx]);

This will produce the same results as the previous example, but it is much
easier to specify/change which elements are to be dumped. One important note
regarding this syntax. Most system tasks/functions keep the variable selection
(for this case it is a variable array word selection) context. If ``$dumpvars``
did this then all callback created would point to this element and would use
the same index which for the example above would have the value 2. This is
certainly not what is desired and for this special case when ``$dumpvars``
executes it uses the current index value to create a constant array selection
and that is monitored instead of the original variable selection.

Referencing Declarations Within an Unnamed Generate Block
---------------------------------------------------------

The IEEE 1364-2005 standard permits generate blocks to be unnamed, but states:

  "If the generate block selected for instantiation is not named, it still
  creates a scope; but the declarations within it cannot be referenced using
  hierarchical names other than from within the hierarchy instantiated by the
  generate block itself."

The standard later defines a scheme for automatically naming the unnamed
scopes for use with external interfaces.

Icarus Verilog implements the defined automatic naming scheme, but does not
prevent the automatically generated names being used in a hierarchical
reference. This behaviour is harmless - the automatically generated names are
guaranteed to be unique within the enclosing scope, so there is no possibility
of confusion with explicit scope names. However, to maintain code portability,
it is recommended that this behavior is not exploited.

``%g/%G`` Format Specifiers
---------------------------

In the IEEE 1364-2001 standard there is a general statement that the real
number format specifiers will use the full formatting capabilities of C.
This is then followed by an example that describes ``%10.3g``. The example
description would be correct for the ``%e`` format specifier which should
always have three fractional digits, but the ``%g`` format specifier does
not work that way. For it the ``.3`` specifies that there will be three
significant digits. What this means is that ``%g`` will always produce one
less significant digit than ``%e`` and will only match the output from ``%f``
for certain values. For example:

.. code-block:: verilog

  module top_level;
    real rval;
    initial begin
      rval = 1234567890;
      $display("This is g and e: %10.3g, %10.3e.", rval, rval);
      rval = 0.1234567890;
      $display("This is g and f: %10.3g, %10.3f.", rval, rval);
      rval = 1.234567890;
      $display("This is more g and f: %10.3g, %10.3f.", rval, rval);
    end
  endmodule // top_level

will produce the following output:

.. code-block:: verilog

 This is g and e:   1.23e+09,  1.235e+09.
 This is g and f:      0.123,      0.123.
 This is more g and f:       1.23,      1.235.

``%t`` Time Format Specifier Can Specify Width
----------------------------------------------

Standard Verilog does not allow width fields in the ``%t`` formats of display
strings. For example, this is illegal:

.. code-block:: verilog

  $display("Time is %0t", $time);

Standard Verilog instead relies on the ``$timeformat`` to completely specify
the format.

Icarus Verilog allows the programmer to specify the field width. The ``%t``
format in Icarus Verilog works exactly as it does in standard Verilog.
However, if the programmer chooses to specify a minimum width (i.e., ``%5t``),
then for that display Icarus Verilog will override the ``$timeformat`` minimum
width and use the explicit minimum width.

``%v`` Format Specifier Can Display Vectors
-------------------------------------------

The IEEE 1364-2005 standard limits the ``%v`` specifier in display strings to
work only with a single bit. Icarus Verilog extends that to support displaying
the strength of vectors. The output is a strength specifier for each bit of the
vector, with underscore characters separating each bit, e.g. ``St0_St1_Pu1_HiZ``.
Most other tools will just print the strength of the least significant bit of
a vector, so this may give different output results for code that otherwise
works fine.

Assign/Deassign and Force/Release of Bit/Part Selects
-----------------------------------------------------

Icarus Verilog allows as an extension the assign/deassign and force/release
of variable bit and part selects in certain cases. This allows the Verilog
test bench writer to assign/deassign for example single bits of a variable
(register, etc.). Other tools will report this as an error.

``repeat`` Statement is Sign Aware
----------------------------------

The standard does not specify what to do for this case, but it does say what
a repeat event control should do. In Icarus Verilog the ``repeat`` statement
is consistent with the repeat event control definition. If the argument is
signed and is a negative value this will be treated the same as an argument
value of 0.

Built-in System Functions May Be Evaluated at Compile Time
----------------------------------------------------------

Certain of the system functions have well-defined meanings, so can
theoretically be evaluated at compile-time, instead of using runtime VPI
code. Doing so means that VPI cannot override the definitions of functions
handled in this manner. On the other hand, this makes them synthesizable,
and also allows for more aggressive constant propagation. The functions
handled in this manner are:

- ``$bits``
- ``$signed``
- ``$sizeof``
- ``$unsigned``

Implementations of these system functions in VPI modules will be ignored.

``vpiScope`` Iterator on ``vpiScope`` Objects
---------------------------------------------

In the VPI, the normal way to iterate over ``vpiScope`` objects contained
within a ``vpiScope`` object, is the ``vpiInternalScope`` iterator. Icarus
Verilog adds support for the ``vpiScope`` iterator of a ``vpiScope`` object,
that iterates over *everything* that is contained in the current scope. This
is useful in cases where one wants to iterate over all the objects in a scope
without iterating over all the contained types explicitly.

Time 0 Race Resolution
----------------------

Combinational logic is routinely modelled using always blocks. However, this
can lead to race conditions if the inputs to the combinational block are
initialized in initial statements. Icarus Verilog slightly modifies time 0
scheduling by arranging for always statements with ANYEDGE sensitivity lists
to be scheduled before any other threads. This causes combinational always
blocks to be triggered when the values in the sensitivity list are initialized
by initial threads.
