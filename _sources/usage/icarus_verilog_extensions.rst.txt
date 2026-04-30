
Icarus Verilog Extensions
=========================

Icarus Verilog supports certain extensions to the baseline IEEE 1364
standard. Some of these are picked from extended variants of the
language, such as SystemVerilog, and some are expressions of internal
behavior of Icarus Verilog, made available as a tool debugging aid.

Don't use any of these extensions if you want to keep your code portable
across other Verilog compilers.

System Functions
----------------

``$is_signed(<expr>)``
^^^^^^^^^^^^^^^^^^^^^^

This function returns 1 if the expression contained is signed, or 0 otherwise.
This is mostly of use for compiler regression tests.

``$bits(<expr>)``, ``$sizeof(<expr>)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``$bits`` system function returns the size in bits of the expression that
is its argument. The result of this function is undefined if the argument
doesn't have a self-determined size.

The ``$sizeof`` system function is deprecated in favour of ``$bits``, which is
the same thing, but included in the SystemVerilog definition.

``$simtime()``
^^^^^^^^^^^^^^

This returns as a 64bit value the simulation time, unscaled by the time units
of the local scope. This is different from the ``$time`` and ``$stime``
functions which return the scaled times. This function is added for regression
testing of the compiler and run time, but can be used by applications who
really want the simulation time.

Note that the simulation time can be confusing if there are lots of different
```timescales`` within a design. It is not in general possible to predict
what the simulation precision will turn out to be.

``$mti_random()``, ``$mti_dist_uniform``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These functions are similar to the IEEE 1364 standard ``$random`` functions,
but they use the Mersenne Twister (MT19937) algorithm. This is considered an
excellent random number generator, but does not generate the same sequence as
the standardized ``$random``.

System Tasks
------------

``$readmempath``
^^^^^^^^^^^^^^^^

The ``$readmemb`` and ``$readmemh`` system tasks read text files that contain
data values to populate memories. Normally, those files are found in a current
working directory. The ``$readmempath()`` system task can be used to create a
search path for those files. For example:

.. code-block:: verilog

  reg [7:0] mem [0:7];
  initial begin
    $readmemh("datafile.txt", mem);
  end

This assumes that "datafile.txt" is in the current working directory where
the ``vvp`` command is running. But with the ``$readmempath``, one can specify
a search path:

.. code-block:: verilog

  reg [7:0] mem [0:7];
  initial begin
    $readmempath(".:alternative:/global/defaults");
    $readmemh("datafile.txt", mem);
  end

In this example, "datafile.txt" is searched for in each of the directories
in the above list (separated by ":" characters). The first located instance
is the one that is used. So for example, if "./datafile.txt" exists, then it
is read instead of "/global/defaults/datafile.txt" even if the latter exists.

``$finish_and_return(code)``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This task operates the same as the ``$finish`` system task, but adds the
feature of specifying an exit code for the interpreter. This can be useful in
automated test environments to indicate whether the simulation finished with
or without errors.

Extended Verilog Data Types
---------------------------

This feature is turned on by the generation flag "-gxtypes" and turned
off by the generation flag "-gno-xtypes". It is turned on by default.

Icarus Verilog adds support for extended data types. This extended
type syntax is based on a proposal by Cadence Design Systems,
originally as an update to the IEEE 1364 standard. Icarus Verilog
currently only takes the new primitive types from the proposal.

SystemVerilog provides the same functionality using somewhat different
syntax. This extension is maintained for backwards compatibility.

- Types

Extended data types separates the concept of net/variable from the
data type. Both nets and variables can declared with any data
type. The primitive types available are::

    logic  - The familiar 0, 1, x and z, optionally with strength.
    bool   - Limited to only 0 and 1
    real   - 64-bit real values

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
types. This extension removes that restriction, allowing any of
the above types to pass through the port consistent with the
continuous assignment connectivity that is implied by the type.

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

Arithmetic operators return real if either of their operands is real,
otherwise they return logic if either of their operands is logic. If
both operands are bool, they return bool.
