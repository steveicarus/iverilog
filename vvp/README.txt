/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *  $Id: README.txt,v 1.54 2005/01/22 01:06:20 steve Exp $
 */

VVP SIMULATION ENGINE

The VVP simulator takes as input source code not unlike assembly
language for a conventional processor. It is intended to be machine
generated code emitted by other tools, including the Icarus Verilog
compiler, so the syntax, though readable, is not necessarily
convenient for humans.


GENERAL FORMAT

The source file is a collection of statements. Each statement may have
a label, an opcode, and operands that depend on the opcode. For some
opcodes, the label is optional (or meaningless) and for others it is
required.

Every statement is terminated by a semicolon. The semicolon is also
the start of a comment line, so you can put comment text after the
semicolon that terminates a statement. Like so:

	Label .functor and, 0x5a, x, y  ; This is a comment.

The semicolon is required, whether the comment is there or not.

Statements may span multiple lines, as long as there is no text (other
then the first character of a label) in the first column of the
continuation line.

HEADER SYNTAX

Before any other non-commentary code starts, the source may contain
some header statements. These are used for passing parameters or
global details from the compiler to the vvp run-time. In all cases,
the header statement starts with a left-justified keyword.

* :module "name" ;

This header statement names a vpi module that vvp should load before
the rest of the program is compiled. The compiler looks in the
standard VPI_MODULE_PATH for files named "name.vpi", and tries to
dynamic load them.

* :vpi_time_precision [+|-]<value>;

This header statement specifies the time precision of a single tick of
the simulation clock. This is mostly used for display (and VPI)
purposes, because the engine itself does not care about units. The
compiler scales time values ahead of time.

The value is the size of a simulation tick in seconds, and is
expressed as a power of 10. For example, +0 is 1 second, and -9 is 1
nano-second. If the record is left out, then the precision is taken to
be +0.

LABELS AND SYMBOLS

Labels and symbols consist of the characters:

	a-z
	A-Z
	0-9
	.$_<>

Labels and symbols may not start with a digit or a '.', so that they
are easily distinguished from keywords and numbers. A Label is a
symbol that starts a statement. If a label is present in a statement,
it must start in the first text column. This is how the lexical
analyzer distinguishes a label from a symbol. If a symbol is present
in a statement, it is in the operand. Opcodes of statements must be a
keyword.

Symbols are references to labels. It is not necessary for a label to
be declared before its use in a symbol, but it must be declared
eventually. When symbols refer to functors, the symbol represents the
vvp_ipoint_t pointer to the output. (Inputs cannot, and need not, be
references symbolically.)

If the functor is part of a vector, then the symbol is the
vvp_ipoint_t for the first functor. The [] operator can then be used
to reference a functor other then the first in the vector.

There are some special symbols that in certain contexts have special
meanings. As inputs to functors, the symbols "C<0>", "C<1>", "C<x>"
and "C<z>" represent a constant driver of the given value.


SCOPE STATEMENTS:


PARAMETER STATEMENTS:

Parameters are named constants within a scope. These parameters have a
type and value, and also a label so that they can be referenced as VPI
objects.

The syntax of a parameter is:

	<label> .param <name>, <type>, <value>;

The <name> is a string that names the parameter. The name is placed in
the current scope as a vpiParameter object. The <type> is one of the
following:

	real          -- The parameter has a real value
	string        -- The parameter has a string value
	[<msb>,<lsb>,<s>]
		      -- The parameter is a vector, with specified
		         indices. The <s> is s or u for signed or
			 unsigned.

The value, then, is appropriate for the data type. For example:

	P_123 .param "hello", string, "Hello, World.";


FUNCTOR STATEMENTS:

A functor statement is a statement that uses the ``.functor''
opcode. Functors are the basic structural units of a simulation, and
include a type (in the form of a truth table) and up to four inputs. A
label is required for functors.

The general syntax of a functor is:

	<label> .functor <type>, symbol_list ;

The symbol list is 4 names of labels of other functors. These connect
inputs of the functor of the statement to the output of other
functors. If the input is unconnected, use a C<?> symbol instead. The
type selects the truth lookup table to use for the functor
implementation. Most of the core gate types have built in tables.

The initial values of all the inputs and the output is x. Any other
value is passed around as run-time behavior. If the inputs have C<?>
symbols, then the inputs are initialized to the specified bit value,
and if this causes the output to be something other then x, a
propagation event is created to be executed at the start of run time.

The strengths of inputs are ignored by functors, and the output has
fixed drive0 and drive1 strengths. So strength information is
typically lost as it passes through functors.

Almost all of the structural aspects of a simulation can be
represented by functors, which perform the very basic task of
combining up to four inputs down to one output.


UDP STATEMENTS:

A UDP statement either defines a User Defined Primitive, or
instantiates a previously defined UDP by creating a UDP functor.  A
UDP functor has as many inputs as the UDP definition requires.

UDPs come in sequential and combinatorial flavors.  Sequential UDPs
carry an output state and can respond to edges at the inputs.  The
output of a combinatorial UDPs is a function of its current inputs
only.

The function of a UDP is defined via a table.  The rows of the table
are strings which describe input states or edges, and the new output
state.	Combinatorial UDPs require one character for each input, and
one character at the end for the output state.	Sequential UDPs need
an additional char for the current state, which is the first char of
the row.

Any input transition or the new state must match at most one row (or
all matches must provide the same output state).  If no row matches,
the output becomes 1'bx.

The output state can be specified as "0", "1", or "x".	Sequential
UDPs may also have "-": no change.

An input or current output state can be

	"1": 1
	"0": 0
	"x": x
	"b": 1, 0
	"h": 1, x
	"l": 0, x
	"?": 1, 0, x

For Sequential UDPs, at most one input state specification may be
replaced by an edge specification.  Valid edges are:

	"*": (??)	"_": (?0)	"+": (?1)	"%": (?x)
	"P": (0?)			"r": (01)	"Q": (0x)
	"N": (1?)	"f": (10)			"M": (1x)
	"B": (x?)	"F": (x0)	"R": (x1)

	"n": (1?) | (?0)
	"p": (0?) | (?1)

A combinatorial UDP is defined like this:

	<type> .udp/comb "<name>", <number>, "<row0>", "<row1>", ... ;

<type> is a label that identifies the UDP.  <number> is the number of
inputs.	 "<name>" is there for public identification.  Sequential UDPs
need an additional initialization value:

	<type> .udp/sequ "<name>", <number>, <init>, "<row0>", "<row1>", ... ;

<init> is the initial value for all instances of the UDP.  We do not
provide initial values for individual instances.  <init> must be a
number 0, 1, or 2 (for 1'bx).

A UDP functor instance is created so:

	<label> .udp  <type>, <symbol_list> ;

Where <label> identifies the functor, <type> is the label of a UDP
defined earlier, and <symbol_list> is a list of symbols, one for each
input of the UDP.


VARIABLE STATEMENTS:

A variable is a bit vector that can be written by behavioral code (so
has no structural input) and propagates its output to a functor. The
general syntax of a variable is:

	<label> .var   "name", <msb>, <lsb>;
	<label> .var/s "name", <msb>, <lsb>;

The "name" is the declared base name of the original variable, for the
sake of VPI code that might access it. The variable is placed in the
current scope. The variable also has a width, defined by the indices
for the most significant and lest significant bits. If the indices are
equal (normally 0) the vector has width of one. If the width is greater
then one, a contiguous array of functors is created and the value of
the label is the address of the least significant bit.

A variable does not take inputs, since its value is set behaviorally
by assignment events. It does have output, though, and its output is
propagated into the net of functors in the usual way.

A variable gets its value by assignments from procedural code: %set
and %assign. These instructions write values to the port-0 input. From
there, the value is held.

Behavioral code can also invoke %cassign/v statements that work like
%set/v, but instead write to port-1 of the variable node. Writes to
port-1 of a variable activate continuous assign mode, where the values
written to port-0 are ignored. The continuous assign mode remains
active until a long(1) is written to port-3 (a command port).

Behavioral code may also invoke %force/v statements that write to port-2
to invoke force mode. This overrides continuous assign mode until a
long(2) is written to port-3 to disable force mode.

NET STATEMENTS:

A net is similar to a variable, except that a thread cannot write to
it (unless it uses a force) and it is given a different VPI type
code. The syntax of a .net statement is also similar to but not
exactly the same as the .var statement:

	<label> .net   "name", <msb>, <lsb>, <symbol>;
	<label> .net/s "name", <msb>, <lsb>, <symbol>;

Like a .var statement, the .net statement creates a VPI object with
the basename and dimensions given as parameters. The symbol is a
functor that feeds into the vector of the net, and the vpiHandle
holds references to that functor.

      NOTE: Nets also, unlike .vars, should also have a way of getting
      at the strengths of each bit. I haven't worked out how that will
      happen, yet.

The input of a .net is replicated to its output. In this sense, it
acts like a diode. The purpose of this node is to hold various VPI
and event trappings.

The <label> is required and is used to locate the net object that is
represents. This label does not map to a functor, so only references
that know they want to access .nets are able to locate the symbol. In
particular, this includes behavioral %load and %wait instructions. The
references to net and reg objects are done through the .net label
instead of a general functor symbol. The instruction stores the
functor pointer, though.


MEMORY STATEMENTS:

Three types of memory statement perform (1) creation of a memory, (2)
connecting a read port to an existing memory, and (3) initializing the
memory's contents.

       <label> .mem "name", <msb>,<lsb>, <last>,<first> ... ;

The pair of numbers <msb>,<lsb> defines the data port width.  The pair
<last>,<first> defines the address range.  Multiple address ranges are
allowed for multidimensional indexing.

Procedural access to the memory references the memory as single array
of bits.  For this purpose, the number of bits in a memory word is
rounded up to the next multiple of four.  That is, for an 18 bit wide
data bus, bit 0 is the lsb of the first word, bit 20 is the lsb of the
second word.

Structural read access is implemented in terms of address and data
ports.  The addresses applied to the address port are expected to be
withing the ranges specified, not based at zero.

A read port is a vector of functors that is wide enough to accept all
provided address bits and at least as wide as the requested subset of
the data port.

	<label> .mem/port <memid>, <msb>,<lsb>, <aw>, <addr_bits> ;

<label> identifies the vector of output functors, to allow connections
to the data output.  <memid> is the label of the memory. <msb>,<lsb>
select a part of the data width.  These are not relative to the data
port range defined for the memory.  The LSB of the data word is here
referred to as 0, regardless to the range specified in the memory
definition.  <addr_bits> is a list of symbols, which connect to the
address inputs of this port.  There are <aw> address bits (<aw> may
become a list of numbers, when multi-index memory ports become
supported).

Any address change, or any change in the addressed memory contents is
immediately propagated to the port outputs.

A write port is a superset of a read port.  It is a vector of functors
that is wide enough to accept the address bits, an event input, a
write enable input, and the data inputs.

	<label> .mem/port <memid>, <msb>,<lsb>, <aw>, <addr_bits>,
                          <event>, <we>, <data> ;

<event> is an event functor that triggers a write, if the <we> input
is true.  <data> is a list of symbols that connect to the data input
port.  For asynchronous transparent write operation, connect
<event> to C<z>, the RAM will transparently follow any changes on
address and data lines, while <we> is true.

There is no Verilog construct that calls for a structural write port
to a memory, but synthesis may ask for lpm_ram_d[pq] objects.

To initialize a memory, use:

	        .mem/init <memid>[<start>],
			val val val ...
			;

<memid> is the label of the memory.  [<start>] is optional,
identifying the bits location where the first value is loaded.
<start> must be a multiple of four, and defaults to zero, if omitted.

The values are decimal or hex numbers (0x prefix), which may be
optionally separated by comma ','.  Each number in the range 0..256
initializes four memory bits.  Two bits form each byte for each memory
bit, in the usual encoding.

Procedural access to the memory employs an index register to address a
bit location in the memory, via the commands:

	%load/m   <bit>, <memid> ;
	%set/m    <memid>, <bit> ;
	%assign/m <memid>, <delay>, <bit> ;

The memory bit is addressed by index register 3.  The value of
register 3 is the index in the memory's bit space, where each data
word occupies a multiple of four bits.


EVENT STATEMENTS

Threads need to interact with the functors of a netlist synchronously,
as well as asynchronously. There are cases where the web of functors
needs to wake up a waiting thread. The web of functors signals threads
through .event objects, that are declared like so:

	<label> .event <type>, <symbols_list>;
	<label> .event "name";


This event statement declares an object that a %wait instruction
can take as an operand. When a thread executes a %wait, it puts
itself in the notification list of the event and suspends. The
<symbols_list> is a set of inputs that can trigger the event.

The <type> describes the conditions needed to trigger the event. It
may be posedge, negedge or edge. If the type is instead a "name"
string, then this is a named event which receives events by the %set
instruction instead of from the output of a functor.

If the event has inputs (a requirement unless it is a named event)
then it has up to 4 symbols that address functors. The event then
detects the appropriate edge on any of the inputs and signals when the
event is true. Normally (in Verilog) a posedge or negedge event only
watches a single bit, so the generated code would only include a
single symbol for the addressed bit. However, if there are several
events of the same edge in an event OR expression, the compiler may
combine up to 4 into a single event.

If many more events need to be combined together (for example due to
an event or expression in the Verilog) then this form can be used:

	<label> .event/or <symbols_list>;

In this case, the symbols list all the events that are to be combined
to trigger this event. Only one of the input events needs to trigger
to make this one go.


WORD STATEMENTS:

Verilog includes some scalar word types available to the programmer,
including real variables, and possible extension types that the code
generator can transparently use. Variables of these special types are
declared with .word statements:

	<label> .word <type>, "vpi name";

The <type> values supported are listed below. The vpi name is the base
name given to the VPI object that is created.

        real  - represents a double precision real variable.


RESOLVER STATEMENTS:

Resolver statements are strength-aware functors with 4 inputs, but
their job typically is to calculate a resolved output using strength
resolution. The type of the functor is used to select a specific
resolution function.

	<label> .resolv tri,  <symbols_list>;
	<label> .resolv tri0, <symbols_list>;
	<label> .resolv tri1, <symbols_list>;

The output from the resolver is vvp_vector8_t value. That is, the
result is a vector with strength included.


PART SELECT STATEMENTS:

Part select statements are functors with three inputs. They take in at
port-0 a vector, and output a selected (likely smaller) part of that
vector. The other inputs specify what those parts are, as a cannonical
bit number, and a width. Normally, those bits are constant values.

	<label> .part <symbol>, <base>, <wid>;
	<label> .part/pv <symbol>, <base>, <wid>, <vector_wid>;

The input is typically a .reg or .net, but can be any vector node in
the netlist.

The .part/pv variation is the inverse of the .part version, in that
the output is actually written to a *part* of the output. The node
uses special part-select-write functions to propagate a part of a
network. The <vector_wid> is the total width of the destination net
that part is written to. Destination nodes use this value to check
further output widths.


PART CONCATENATION STATEMENTS:

The opposite of the part select statement is the part concatenation
statement. The .concat statment is a functor node that takes at input
vector values and produces a single vector output that is the
concatenation of all the inputs.

        <label> .concat [W X Y Z], <symbols_list> ;

The "[" and "]" tokens surround a set of 4 numbers that are the
expected widths of all the inputs. These widths are needed to figure
the positions of the input vectors in the generated output, and are
listed in order LSB to MSB. The inputs themselves are also listed LSB
to MSB, with the LSB vector input coming through port-0 of the real
functor.

The initial output value is (W+X+Y+Z) bits of 'bx. As input values are
propagated, the bits are placed in the correct place in the output
vector value, and a new output value is propagated.

FORCE STATEMENTS (old method - remove me):

A force statement creates functors that represent a Verilog force
statement.

	<label>	.force <signal>, <symbol_list>;

The symbol <signal> represents the signal which is to be forced.  The
<symbol_list> specifies the bits of the expression that is to be
forced on the <signal>.  The <label> identifies the force functors.
There will be as many force functors as there are symbols in the
<symbol_list>.

To activate and deactivate a force on a single bit, use:

	%force	<label>, <width>;
	%release <signal>;

<label>/<width> is the label/width of a vector of force functors.
<signal> is the label of the functor that drives the signal that is
being forced.

FORCE STATEMENTS (new method - implement me):

A %force instruction, as described in the .var section, forces a
constant value onto a .var or .net, and the matching %release release
that value. However, there are times when the value of a functor
(i.e. another .net) needs to be forced onto a .var or .net. For this
task, the %force/link instruction exists:

	%force/link <dst>, <src> ;
	%release/link <dst> ;

This causes the output of the node <src> to be linked to the force
input of the <dst> .var/.net node. When linked, the output functor
will automatically drive values to the force port of the destination
node. The matching %release/link instruction removes the link (a
%release is still needed) to the destination.

The instructions:

	%cassign/link <dst>, <src> ;
	%deassign/link <dst> ;

are the same concept, but for the continuous assign port.

STRUCTURAL ARITHMETIC STATEMENTS:

The various Verilog arithmetic operators (+-*/%) ar avaiable to
structural contexts as two-input functors that take in vectors. All of
these operators take two inputs and generate a fixed width output. The
input vectors will be padded if needed to get the desired output width.

	<label> .arith/sub  <wid>, <A>, <B>;
	<label> .arith/sum  <wid>, <A>, <B>;
	<label> .arith/mult <wid>, <A>, <B>;
	<label> .arith/div  <wid>, <A>, <B>;
	<label> .arith/mod  <wid>, <A>, <B>;

In all cases, there are no width limits, so long as the width is fixed.

STRUCTURAL COMPARE STATEMENTS:

The arithmetic statements handle various arithmetic operators that
have wide outputs, but the comparators have single bit output, so they
are implemented a bit differently. The syntax, however, is very
similar:

	<label> .cmp/eeq <wid>, <A>, <B>;
	<label> .cmp/eq  <wid>, <A>, <B>;
	<label> .cmp/ne  <wid>, <A>, <B>;
	<label> .cmp/ge  <wid>, <A>, <B>;
	<label> .cmp/gt  <wid>, <A>, <B>;
	<label> .cmp/ge.s <wid>, <A>, <B>;
	<label> .cmp/gt.s <wid>, <A>, <B>;

Whereas the arithmetic statements generate an output the width of
<wid>, the comparisons produce a single bit vector result. The plain
versions do unsigned comparison, but the ".s" versions to signed
comparisons. (Equality doesn't need to care about sign.)


STRUCTURAL SHIFTER STATEMENTS:

Variable shifts in structural context are implemented with .shift
statements:

	<label> .shift/sl <wid>, <symbols_list>;

The shifter has a width that defines the number of outputs and the
number of data inputs. The first <wid> symbols in the <symbols_list>
gives the input data. The remaining hold the shift value, lsb first.


STRUCTURAL FUNCTION CALLS:

The .ufunc statement defines a call to a user defined function.

	<label> .ufunc ;


THREAD STATEMENTS:

Thread statements create the initial threads for a simulation. These
represent the initial and always blocks, and possibly other causes to
create threads at startup.

	.thread <symbol> [, <flag>]

This statement creates a thread with a starting address at the
instruction given by <symbol>. When the simulation starts, a thread is
created for the .thread statement, and it starts at the <symbol>
addressed instruction.

The <flag> modifies the creation/execution behavior of the
thread. Supported flags are:

	$push -- Cause the thread to be pushed in the scheduler. This
		 only effects startup (time 0) by arranging for pushed
		 threads to be started before non-pushed threads. This
		 is useful for resolving time-0 races.

* Threads in general

Thread statements create the initial threads of a design. These
include the ``initial'' and ``always'' statements of the original
Verilog, and possibly some other synthetic threads for various
purposes. It is also possible to create transient threads from
behavioral code. These are needed to support such constructs as
fork/join, named blocks and task activation.

A transient thread is created with a %fork instruction. When a
transient thread is created this way, the operand to the %fork gives
the starting address, and the new thread is said to be a child of the
forking thread. The children of a thread are pushed onto a stack of
children. A thread can have only one direct child.

A transient thread is reaped with a %join instruction. %join waits for
the top thread in the stack of children to complete, then
continues. It is an error to %join when there are no children.

As you can see, the transient thread in VVP is a cross between a
conventional thread and a function call. In fact, there is no %call
instruction in vvp, the job is accomplished with %fork/%join in the
caller and %end in the callee. The %fork, then is simply a
generalization of a function call, where the caller does not
necessarily wait for the callee.

For all the behavior of threads and thread parentage to work
correctly, all %fork statements must have a corresponding %join in the
parent, and %end in the child. Without this proper matching, the
hierarchical relationships can get confused. The behavior of erroneous
code is undefined.

* Thread Context

The context of a thread is all the local data that only that thread
can address. The local data is broken into two addresses spaces: bit
memory and word memory.

The bit memory is a region of 4-value bits (0,1,x,z) that can be
addressed in strips of arbitrary length. For example, an 8-bit value
can be in locations 8 through and including 15. The bits at address 0,
1, 2 and 3 are special constant values. Reads from those locations
make vectors of 0, 1, x or z values, so these can be used to
manufacture complex values elsewhere.

The word memory is a region of tagged words. The value in each word
may be native long or real. These words have a distinct address space
from the bits.

* Threads and scopes

The Verilog ``disable'' statement deserves some special mention
because of how it interacts with threads. In particular, threads
throughout the design can affect (end) other threads in the design
using the disable statement.

In Verilog, the operand to the disable statement is the name of a
scope. The behavior of the disable is to cause all threads executing
in the scope to end. Termination of a thread includes all the children
of the thread. In vvp, all threads are in a scope, so this is how the
disable gains access to the desired thread.

It is obvious how initial/always thread join a scope. They become part
of the scope simply by being declared after a .scope declaration. (See
vvp.txt for .scope declarations.) The .thread statement placed in the
assembly source after a .scope statement causes the thread to join the
named scope.

Transient threads join a scope that is the operand to the %fork
instruction. The scope is referenced by name, and the thread created
by the fork atomically joins that scope. Once the transient thread
joins the scope, it stays there until it ends. Threads never change
scopes, not even transient threads.

TRUTH TABLES

The logic that a functor represents is expressed as a truth table. The
functor has four inputs and one output. Each input and output has one
of four possible values (0, 1, x and z) so two bits are needed to
represent them. So the input of the functor is 8 bits, and the output
2 bits. A complete lookup table for generating the 2-bit output from
an 8-bit input is 512 bits. That can be packed into 64 bytes. This is
small enough that the table should take less space then the code to
implement the logic.

To implement the truth table, we need to assign 2-bit encodings for
the 4-value signals. I choose, pseudo-randomly, the following
encoding:

	1'b0  : 00
	1'b1  : 01
	1'bx  : 10
	1'bz  : 11

The table is an array of 64 bytes, each byte holding 4 2-bit
outputs. Construct a 6-bit byte address with inputs 1, 2 and 3 like
so:
	 332211

The input 0 2-bits can then be used to select which of the 4 2-bit
pairs in the 8-bit byte are the output:

	MSB -> zzxx1100 <- LSB

A complete truth table, then is described as 64 8-bit bytes.

The vvp engine includes truth tables for the primitive gate types, so
none needs to be given by the programmer. It is sufficient to name the
type to get that truth table.


EXECUTABLE INSTRUCTIONS

Threads run executable code, much like a processor executes machine
code. VVP has a variety of opcodes for executable instructions. All of
those instructions start with '%' and go into a single address
space. Labels attached to executable instructions get assigned the
address of the instruction, and can be the target of %jmp instructions
and starting points for threads.

The opcodes.txt file has a more detailed description of all the
various instructions.


THE RELATIONSHIP BETWEEN FUNCTORS, THREADS AND EVENTS

Given the above summary of the major components of vvp, some
description of their relationship is warranted. Functors provide a
structural description of the design (so far as it can be described
structurally) and these functors run independently of the threads. In
particular, when an input to a functor is set, it calculates a new
output value; and if that output is different from the existing
output, a propagation event is created. Functor output is calculated
by truth table lookup, without the aid of threads.

Propagation events are one of three kinds of events in vvp. They are
scheduled to execute at some time, and they simply point to the functor
that is to have its output propagated. When the event expires, the
output of the referenced functor is propagated to all the inputs that
it is connected to, and those functors in turn create new events if
needed.

Assignment events (the second of three types of events) are created
by non-blocking assignments in behavioral code. When the ``<='' is
executed (a %assign in vvp) an assign event is created, which includes
the vvp_ipoint_t pointer to the functor input to receive the value,
as well as the value. These are distinct from propagation events because:

	a) There is no functor that has as its output the value to be
	   assigned (this is how values get into the functor net in
	   the first place), and

	b) This allows for behavioral code to create waveforms of
	   arbitrary length that feed into a variable. Verilog allows
	   this of non-blocking assignments, but not of gate outputs.

The last type of event is the thread schedule event. This event simply
points to a thread to be executed. Threads are made up of a virtual
processor with a program counter and some private storage. Threads
can execute %assign instructions to create assignment events, and can
execute %set instructions to do blocking assignments. Threads can also
use %load to read the output of functors.

The core event scheduler takes these three kinds of events and calls
the right kind of code to cause things to happen in the design. If the
event is a propagate or assignment event, the network of functors is
tickled; if the event is a thread schedule, then a thread is run. The
implementation of the event queue is not important, but currently is
implemented as a ``skip list''. That is, it is a sorted singly linked
list with skip pointers that skip over delta-time events.

The functor net and the threads are distinct. They communicate through
thread instructions %set, %assign, %waitfor and %load. So far as a thread
is concerned, the functor net is a blob of structure that it pokes and
prods via certain functor access instructions.


VVP COMPILATION AND EXECUTION

The vvp program operates in a few steps:

	1) Initialization
		Data structures are cleared to empty, and tables are
		readied for compilation.

	2) Compilation
		The input file is read and compiled. Symbol tables are
		build up as needed, objects are allocated and linked
		together.

	3) Cleanup
		Symbol tables and other resources used only for
		compilation are released to reduce the memory
		footprint.

	4) Simulation
		Event simulation is run.


The initialization step is performed by the compile_init() function in
compile.cc. This function in turn calls all the *_init() functions in
other parts of the source that need initialization for compile. All
the various sub-init functions are called <foo>_init().

Compilation is controlled by the parser, it parse.y. As the parser
reads and parses input, the compilation proceeds in the rules by
calling various compile_* functions. All these functions live in the
compile.cc file. Compilation calls other sections of the code as
needed.

When the parser completes compilation, compile_cleanup() is called to
finish the compilation process. Unresolved references are completed,
then all the symbol tables and other compile-time-only resources are
released. Once compile_cleanup() returns, there is no more use for the
parser for the function in compile.cc.

After cleanup, the simulation is started. This is done by executing
the schedule_simulate() function. This does any final setup and starts
the simulation running and the event queue running.


HOW TO GET FROM THERE TO HERE

The vvp simulation engine is designed to be able to take as input a
compiled form of Verilog. That implies that there is a compiler that
compiles Verilog into a form that the vvp engine can read.


* Boolean logic gates

Gates like AND, OR and NAND are implemented simply and obviously by
functor statements. Any logic up to 4 inputs can be implemented with a
single functor. For example:

	and gate (out, i1, i2, i3);

becomes:

	gate	.functor and, i1, i2, i3;

Notice the first parameter of the .functor is the type. The type
includes a truth table that describes the output with a given
input. If the gate is wider then four inputs, then cascade
functors. For example:

	and gate (out, i1, i2, i3, i4, i5, i6, i7, i8);

becomes:

	gate.0	.functor and, i1, i2, i3, i4;
	gate.1	.functor and, i5, i6, i7, i8;
	gate	.functor and, gate.0, gate.1;


* reg and other variables

Reg and integer are cases of what Verilog calls ``variables.''
Variables are, simply put, things that behavioral code can assign
to. These are not the same as ``nets,'' which include wires and the
like.

Each bit of a variable is created by a ``.var'' statement. For example:

	reg a;

becomes:

	a	.var "a", 0, 0;


* named events

Events in general are implemented as functors, but named events in
particular have no inputs and only the event output. The way to
generate code for these is like so:

	a  .event "name";

This creates a functor and makes it into a mode-2 functor. Then the
trigger statement, "-> a", cause a ``%set a, 0;'' statement be
generated. This is sufficient to trigger the event.

/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
