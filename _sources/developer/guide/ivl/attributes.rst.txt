
Icarus Verilog Attributes
=========================

Attribute Naming Conventions
----------------------------

Attributes that are specific to Icarus Verilog, and are intended to be
of use to programmers, start with the prefix "ivl\_".

Attributes with the "_ivl_" prefix are set aside for internal
use. They may be generated internally by the compiler. They need not
be documented here.

Attributes To Control Synthesis
-------------------------------

The following is a summary of Verilog attributes that Icarus Verilog
understands within Verilog source files to control synthesis
behavior. This section documents generic synthesis attributes. For
target specific attributes, see target specific documentation.

These attributes only effect the behavior of the synthesizer. For
example, the ivl_combinational will not generate an error message
if the Verilog is being compiled for simulation. (It may generate a
warning.)


* Attributes for "always" and "initial" statements

(\* ivl_combinational \*)

    This attribute tells the compiler that the statement models
    combinational logic. If the compiler finds that it cannot make
    combinational logic out of a marked always statement, it will
    report an error.

    This attribute can be used to prevent accidentally inferring
    latches or flip-flops where the user intended combinational
    logic.

(\* ivl_synthesis_on \*)

    This attribute tells the compiler that the marked always statement
    is synthesizable. The compiler will attempt to synthesize the
    code in the marked "always" statement. If it cannot in any way
    synthesize it, then it will report an error.

(\* ivl_synthesis_off \*)

    If this value is attached to an "always" statement, then the
    compiler will *not* synthesize the "always" statement. This can be
    used, for example, to mark embedded test bench code.


* Attributes for modules

(\* ivl_synthesis_cell \*)

    If this value is attached to a module during synthesis, that
    module will be considered a target architecture primitive, and
    its interior will not be synthesized further.  The module can
    therefore hold a model for simulation purposes.


* Attributes for signals (wire/reg/integer/tri/etc.)

(\* PAD = "<pad assignment list>" \*)

    If this attribute is attached to a signal that happens to be a
    root module port, then targets that support it will use the string
    value as a list of pin assignments for the port/signal. The format
    is a comma separated list of location tokens, with the format of
    the token itself defined by the back-end tools in use.

* Other Attributes

[ none defined yet ]


Misc
----

(\* _ivl_schedule_push \*)

    If this attribute is attached to a thread object (always or
    initial statement) then the vvp code generator will generate code
    that causes the scheduler to push this thread at compile time. The
    compiler may internally add this attribute to always statements if
    it detects that it is combinational. This helps resolve time-0
    races.
