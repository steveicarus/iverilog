
Icarus Verilog Extensions
=========================

Icarus Verilog supports certain extensions to the baseline IEEE1364
standard. Some of these are picked from extended variants of the
language, such as SystemVerilog, and some are expressions of internal
behavior of Icarus Verilog, made available as a tool debugging aid.

* Builtin System Functions

** Extended Verilog Data Types

This feature is turned off if the generation flag "-g" is set to other
then the default "2x".  For example, "iverilog -g2x" enables extended
data types, and "iverilog -g2" disables them.

Icarus Verilog adds support for extended data types. This extended
type syntax is based on a proposal by Cadence Design Systems,
originally as an update to the IEEE1364. That original proposal has
apparently been absorbed by the IEEE1800 SystemVerilog
standard. Icarus Verilog currently only takes the new primitive types
from the proposal.

Extended data types separates the concept of net/variable from the
data type. Both nets and variables can declared with any data
type. The primitive types available are::

    logic  - The familiar 0, 1, x and z, optionally with strength.
    bool   - Limited to only 0 and 1
    real   - 64bit real values

Nets with logic type may have multiple drivers with strength, and the
value is resolved the usual way. Only logic values may be driven to
logic nets, so bool values driven onto logic nets are implicitly
converted to logic.

Nets with any other type may not have multiple drivers. The compiler
should detect the multiple drivers and report an error.

- Declarations

The declaration of a net is extended to include the type of the wire,
with the syntax::

    wire <type> <wire-assignment-list>... ;

The <type>, if omitted, is taken to be logic. The "wire" can be any of
the net keywords. Wires can be logic, bool, real, or vectors of logic
or bool. Some valid examples::

    wire real foo = 1.0;
    tri logic bus[31:0];
    wire bool addr[23:0];
    ... and so on.

The declarations of variables is similar. The "reg" keyword is used to
specify that this is a variable. Variables can have the same data
types as nets.

- Ports

Module and task ports in standard Verilog are restricted to logic
types. This extension removes that restriction, allowing any type to
pass through the port consistent with the continuous assignment
connectivity that is implied by the type.

- Expressions

Expressions in the face of real values is covered by the baseline
Verilog standard.

The bool type supports the same operators as the logic type, with the
obvious differences imposed by the limited domain.

Comparison operators (not case compare) return logic if either of
their operands is logic. If both are bool or real (including mix of
bool and real) then the result is bool. This is because comparison of
bools and reals always return exactly true or false.

Case comparison returns bool. This differs from baseline Verilog,
which strictly speaking returns a logic, but only 0 or 1 values.

All the arithmetic operators return bool if both of their operands are
bool or real. Otherwise, they return logic.
