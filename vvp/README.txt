/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *  $Id: README.txt,v 1.13 2001/04/05 01:34:26 steve Exp $
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
the header statement starts with a left-justified keyword, and ends
with a string.

* :module "name" ;

This header statement names a vpi module that vvp should load before
the rest of the program is compiled. The compiler looks in the
standard VPI_MODULE_PATH for files named "name.vpi", and tries to
dynamic load them.

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


FUNCTOR STATEMENTS:

A functor statement is a statement that uses the ``.functor''
opcode. Functors are the basic structural units of a simulation, and
include a type (in the form of a truth table) and up to four inputs. A
label is required for functors.

The general syntax of a functor is:

	<label> .functor <type>, <init> [, symbol_list]

The symbol list is 0-4 names of labels of other functors. These
connect inputs of the functor of the statement to the output of other
functors. The type is the label of a .ftype statement elsewhere in the
program. The references .ftype describes the behavoir of the
functor.

The <init> value is the 8-bit initial value of the 4 input ports. The
LSB is port 0, and the MSB port 3.

Almost all of the structural aspects of a simulation can be
represented by functors, which perform the very basic task of
combining up to four inputs down to one output.

VARIABLE STATEMENTS:

A variable is a bit vector that can be written by behavioral code (so
has no structural input) and propagates its output to a functor. The
general syntax of a variable is:

	<label> .var   "name", <msb>, <lsb>;
	<label> .var/s "name", <msb>, <lsb>;

The "name" is the declared base name of the original variable, for the
sake of VPI code that might access it. The variable is placed in the
current scope. The variable also has a width, defined by the indices
for the mst significant and lest significant bits. If the indices are
equal (normally 0) the vector has width of one. If the width is greater
then one, a contiguous array of functors is created and the value of
the label is the address of the least significant bit.

A variable does not take inputs, since its value is set behaviorally
by assignment events. It does have output, though, and its output is
propagated into the net of functors in the usual way.

Therefore, the .var statement implicitly also creates .functors
addressed by the label of the variable. It is in fact the functors
that behavioral code reads when the value of the variable (or net) is
read by behavioral code. If the .var represents a vector of .functors,
the index of the LSB is always, from the perspective of vvp, ZERO. The
<msb>,<lsb> details are there only for the benefit of VPI support.

The variable .functor implicitly has three inputs. The first is the
value that gets set by assignments or procedural continuous
assignments. The second is a forced value that can be connected to a
force expression (as a functor) when a value is being forced. And the
third input selects the source to use. The default is to select the
assignment input.

The variable statement also creates a VPI object of the appropriate
type. See the vpi.txt file for details about that object. The msb and
lsb values are set from the parameters of the .var or .var/s, and the
vpiReg is marked unsigned for .var, or signed for .var/s

Note that nets in a design do not necessarily have a specific functor
or object allocated to them. Nets are just something that behavioral
code can read, so it is enough to give to the behavioral code the
vvp_ipoint_t object of the .functor that drives the net.


NET STATEMENTS:

A net is similar to a variable, except that a thread cannot write to
it (unless it uses a force) and it is given a different VPI type
code. The syntax of a .net statement is also similar to but not
exactly the same as the .var statement:

	<label> .net   "name", <msb>, <lsb>, <symbols_list>;
	<label> .net/s "name", <msb>, <lsb>, <symbols_list>;

A .net statement creates a functor for each bit of the vector in
exactly the same way that a .var creates functors. The truth table for
a .net functor is the same, as well. The net has an output that may be
connected to something, but need not. The 0 (zero) input of each bit
functor is the normal input, and is connected to the output of a
functor or another net.

The second functor input is a force net. This connects to an
expression that is announced by a force statement of some sort. The
third input is the only input settable by behavioral code, it selects
between the normal input and the force input. These inputs work
exactly the same as those for the .var functors.

Since .var items are written by behavioral code, there is no worry
about how to connect their inputs. You don't. But .net items do have
inputs that need to be connected to the outputs of other
functors. Nets are also complicated by the fact that they come in
vectors, so the inputs of all the functors in the array need to be
connected. Fortunately, there is only one input per .net functor that
can be connected, and that is input 0.

The force input and selector input are manipulated exactly the same
way as with .var force and selector inputs, so there is no need for a
syntax to handle them.

Since there is exactly one input per bit in the vector, it is easy
enough to just list the vvp_ipoint_t symbols in order. Each symbol
references a functor, and connects the corresponding .net input to the
output of that functor, as if the .net is an <N> bit wide functor. The
bits of the vector are connected least-significant-bit first. It is
legal to leave a bit unconnected. To do that, simply leave the
position for that bit blank. Bits of .nets are initialized to
z. Unconnected bits keep the value z throughout the simulation.


EVENT STATEMENTS

Threads need to interact with the functors of a netlist synchronously,
as well as asynchronously. There are cases where the web of functors
needs to wake up a waiting thread. The web of functors signals threads
through .event objects, that are declare like so:

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


THREAD STATEMENTS:

Thread statements create the initial threads for a simulation. These
represent the initial and always blocks, and possibly other causes to
create threads at startup.

	.thread <symbol>

This statement creates a thread with a starting address at the
instruction given by <symbol>.


THREADS IN GENERAL:

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
description of their relationship is warrented. Functors provide a
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
partinular have no inputs and only the event output. The way to
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
