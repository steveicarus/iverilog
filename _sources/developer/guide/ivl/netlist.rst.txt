
Netlist Format
==============

The output from the parse and elaboration steps is a "netlist" rooted
in a Design object. Parsing translates the design described in the
initial source file into a temporary symbolic "pform". Elaboration
then expands the design, resolving references and expanding
hierarchies, to produce a flattened netlist. This is the form that
optimizers and code generators use.

The design optimization processes all manipulate the netlist,
translating it to a (hopefully) better netlist after each step. The
complete netlist is then passed to the code generator, the emit
function, where the final code (in the target format) is produced.

Structural Items: NetNode and NetNet
------------------------------------

Components and wires, memories and registers all at their base are
either NetNode objects or NetNet objects. Even these classes are
derived from the NetObj class.

All NetNode and NetNet objects have a name and some number of
pins. The name usually comes from the Verilog source that represents
that object, although objects that are artifacts of elaboration will
have a generated (and probably unreadable) name. The pins are the
ports into the device. NetNode objects have a pin for each pin of the
component it represents, and NetNet objects have a pin for each signal
in the vector.

Node and net pins can be connected together via the connect
function. Connections are transitive (A==B and B==c means A==C) so
connections accumulate on a link as items are connected to it. The
destructors for nets and nodes automatically arrange for pins to be
disconnected when the item is deleted, so that the netlist can be
changed during processing.

Structural Links
----------------

The NetNode and NetNet classes contain arrays of Link objects, one
object per pin. Each pin is a single bit. The Link objects link to all
the NetNode and NetNet objects' links that are connected together in
the design, and to a Nexus object. This way, code that examines a node
of the design can discover what is connected to each pin.

The connected set of links also has common properties that are stored
or access from the Nexus object. All the Links that are connected
together are also connected to a single Nexus object. This object is
useful for accessing the properties and values that come from the
connected set of links. The Nexus object is also handy for iterating
over the connected set of Links.

See the Link class definition in netlist.h for a description of the link
methods, and the Nexus class for nexus global methods.

Currently, a link has 3 possible direction properties:

	PASSIVE -- These pins are sampled by the object that holds the
		   pin based on some external event. These are used,
		   for example, by NetESignal objects that read a
		   point for a procedural expression.

	INPUT   -- These pins potentially react to the setting of its
		   input.

	OUTPUT  -- These pins potentially drive the node. (They may be
		   three-state.)


Behavioral Items: NetProcTop, NetProc and derived classes
---------------------------------------------------------

Behavioral items are not in general linked to the netlist. Instead,
they represent elaborated behavioral statements. The type of the object
implies what the behavior of the statement does. For example, a
NetCondit object represents an `if` statement, and carries a
condition expression and up to two alternative sub-statements.

At the root of a process is a NetProcTop object. This class carries a
type flag (initial or always) and a single NetProc object. The
contained statement may, depending on the derived class, refer to
other statements, compound statements, so on. But at the root of the
tree is the NetProcTop object. The Design class keeps a list of the
elaborated NetProcTop objects. That list represents the list of
processes in the design.

Interaction Of Behavioral And Structural: NetAssign\_
-----------------------------------------------------

The behavioral statements in a Verilog design effect the structural
aspects through assignments to registers. Registers are structural
items represented by the NetNet class, linked to the assignment
statement through pins. This implies that the l-value of an assignment
is structural. It also implies that the statement itself is
structural, and indeed it is derived from NetNode.

The NetAssign\_ class is also derived from the NetProc class because
what it does is brought on by executing the process. By multiple
inheritance we have therefore that the assignment is both a NetNode
and a NetProc. The NetAssign\_ node has pins that represent the l-value
of the statement, and carries behavioral expressions that represent
the r-value of the assignment.

Memories
--------

The netlist form includes the NetMemory type to hold the content of a
memory. Instances of this type represent the declaration of a memory,
and occur once for each memory. References to the memory are managed
by the NetEMemory and NetAssignMem\_ classes.

An instance of the NetEMemory class is created whenever a procedural
expression references a memory element. The operand is the index to
use to address (and read) the memory.

An instance of the NetAssignMem\_ class is created when there is a
procedural assignment to the memory. The NetAssignMem\_ object
represents the l-value reference (a write) to the memory. As with the
NetEMemory class, this is a procedural reference only.

When a memory reference appears in structural context (i.e. continuous
assignments) elaboration creates a NetRamDq. This is a LPM_RAM_DQ
device. Elaboration leaves the write control and data input pins
unconnected for now, because memories cannot appear is l-values of
continuous assignments. However, the synthesis functor may connect
signals to the write control lines to get a fully operational RAM.

By the time elaboration completes, there may be many NetAssignMem\_,
NetEMemory and NetRamDq objects referencing the same NetMemory
object. Each represents a port into the memory. It is up to the
synthesis steps (and the target code) to figure out what to do with
these ports.

Expressions
-----------

Expressions are represented as a tree of NetExpr nodes. The NetExpr
base class contains the core methods that represent an expression
node, including virtual methods to help with dealing with nested
complexities of expressions.

Expressions (as expressed in the source and p-form) may also be
elaborated structurally, where it makes sense. For example, assignment
l-value expressions are represented as connections to pins. Also,
continuous assignment module items are elaborated as gates instead of
as a procedural expression. Event expressions are also elaborated
structurally as events are like devices that trigger behavioral
statements.

However, typical expressions the behavioral description are
represented as a tree of NetExpr nodes. The derived class of the node
encodes what kind of operator the node represents.

Expression Bit Width
--------------------

The expression (represented by the NetExpr class) has a bit width that
it either explicitly specified, or implied by context or contents.
When each node of the expression is first constructed during
elaboration, it is given, by type and parameters, an idea what its
width should be. It certain cases, this is definitive, for example
with signals. In others, it is ambiguous, as with unsized constants.

As the expression is built up by elaboration, operators that combine
expressions impose bit widths of the environment or expose the bit
widths of the sub expressions. For example, the bitwise AND (&)
operator has a bit size implied by its operands, whereas the
comparison (==) operator has a bit size of 1. The building up of the
elaborated expression checks and adjusts the bit widths as the
expression is built up, until finally the context of the expression
takes the final bit width and makes any final adjustments.

The NetExpr::expr_width() method returns the calculated (or guessed)
expression width. This method will return 0 until the width is set by
calculation or context. If this method returns false, then it is up to
the context that wants the width to set one. The elaboration phase
will call the NetExpr::set_width method on an expression as soon as it
gets to a point where it believes that it knows what the width should
be.

The NetExpr::set_width(unsigned) virtual method is used by the context
of an expression node to note to the expression that the width is
determined and please adapt. If the expression cannot reasonably
adapt, it will return false. Otherwise, it will adjust bit widths and
return true.

::

    I do not yet properly deal with cases where elaboration knows for
    certain that the bit width does not matter. In this case, I
    really should tell the expression node about it so that it can
    pick a practical (and optimal) width.

Interaction Of Expressions And Structure: NetESignal
----------------------------------------------------

The NetAssign\_ class described above is the means for processes to
manipulate the net, but values are read from the net by NetESignal
objects. These objects are class NetExpr because they can appear in
expressions (and have width). They are not NetNode object, but hold
pointers to a NetNet object, which is used to retrieve values with the
expression is evaluated.


Hierarchy In Netlists
---------------------

The obvious hierarchical structure of Verilog is the module. The
Verilog program may contain any number of instantiations of modules in
order to form an hierarchical design. However, the elaboration of the
design into a netlist erases module boundaries. Modules are expanded
each place they are used, with the hierarchical instance name used to
name the components of the module instance. However, the fact that a
wire or register is a module port is lost.

The advantage of this behavior is first the simplification of the
netlist structure itself. Backends that process netlists only need to
cope with a list of nets, a list of nodes and a list of
processes. This eases the task of the backend code generators.

Another advantage of this flattening of the netlist is that optimizers
can operate globally, with optimizations freely crossing module
boundaries. This makes coding of netlist transform functions such as
constant propagation more effective and easier to write.


Scope Representation In Netlists
--------------------------------

In spite of the literal flattening of the design, scope information is
preserved in the netlist, with the NetScope class. The Design class
keeps a single pointer to the root scope of the design. This is the
scope of the root module. Scopes that are then created within that
(or any nested) module are placed as children of the root scope, and
those children can have further children, and so on.

Each scope in the tree carries its own name, and its relationship to
its parent and children. This makes it possible to walk the tree of
scopes. In practice, the walking of the scopes is handled by recursive
methods.

Each scope also carries the parameters that are applicable to the
scope itself. The parameter expression (possibly evaluated) can be
located by name, given the scope object itself. The scan of the pform
to generate scopes also places the parameters that are declared in the
scope. Overrides are managed during the scan, and once the scan is
complete, defparam overrides are applied.


Tasks In Netlists
-----------------

The flattening of the design does not include tasks and named
begin-end blocks. Tasks are behavioral hierarchy (whereas modules are
structural) so do not easily succumb to the flattening process. In
particular, it is logically impossible to flatten tasks that
recurse. (The elaboration process does reserve the right to flatten
some task calls. C++ programmers recognize this as inlining a task.)


Time Scale In Netlists
----------------------

The Design class and the NetScope classes carry time scale and
resolution information of the elaborated design. There is a global
resolution, and there are scope specific units and resolutions. Units
and resolutions are specified as signed integers, and interpreted as
the power of 10 of the value. For example, a resolution "-9" means
that "1" is 1ns (1e-9). The notation supports units from -128 to +127.
It is up to the back-ends to interpret "-4" as "100us".

Delays are expressed in the netlist by integers. The units of these
delays are always given in the units of the design precision. This
allows everything to work with integers, and generally places the
burden of scaling delays into elaboration. This is, after all, a
common task. The Design::get_precision() method gets the global design
precision.

Each NetScope also carries its local time_units and time_precision
values. These are filled in during scope elaboration and are used in
subsequent elaboration phases to arrange for scaling of delays. This
information can also be used by the code generator to scale times back
to the units of the scope, if that is desired.
