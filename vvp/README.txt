
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
then the first character of a label) in the first column of hte
continuation line.


LABELS AND SYMBOLS

Labels and symbols consist of the characters:

	a-z
	A-Z
	0-9
	.$_

Labels and symbols may not start with a digit or a '.', so that they
are easily distinguished from keywords and numbers. A Label is a
symbol that starts a statement. If a label is present in a statement,
it must start in the first text column. This is how the lexical
analyzer distinguishes a label from a symbol. If a symbol is present
in a statement, it is in the operand. Opcodes of statements must be a
keyword.

Symbols are references to labels. It is not necessary for a label to
be declared before its use in a symbol, but it must be declared
eventually.


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

A variable is a bit that can be written by behavioral code (so has no
structural input) and propagates its output to a functor. The general
syntax of a variable is:

	<label> .var

A variable does not take inputs, since its value is set behaviorally
by assignment events. It does have an output, though, and its output
is propagated into the net of functors in the usual way.

Therefore, the .var statement implicitly also creates a .functor of
the same name as the variable. It is in fact the functor that
behavioral code reads when the value of the variable (or net) is read
by behavioral code.

The variable .functor implicitly has three inputs. The first is the
value that gets set by assignments or procedural continuous
assignments. The second is a forced value that can be connected to a
force expression (as a functor) when a value is being forced. And the
third input selects the source to use. The default is to select the
assignment input.

Note that nets in a design do not necessarily have a specific functor
or object allocated to them. Nets are just something that behavioral
code can read, so it is enough to give to the behavioral code the
vvp_ipoint_t object of the .functor that drives the net.


THREAD STATEMENTS:

Thread statements create the initial threads for a simulation. These
represent the initial and always blocks, and possibly other causes to
create threads at startup.

	.thread <symbol>

This statement creates a thread with a starting address at the
instruction given by <symbol>.


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

	a	.var;
