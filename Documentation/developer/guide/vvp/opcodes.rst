Executable Instruction Opcodes
==============================

Instruction opcodes all start with a % character and have 0 or more
operands. In no case are there more than 3 operands. This chapter
describes the specific behavior of each opcode, in enough detail
(I hope) that its complete effect can be predicted.

General Principles of Arithmetic (current plan):

The binary arithmetic instruction in general takes three parameters,
the left operand, the right operand, and the base. The left operand is
replaced with the result, which is the same width as the left and
right operands.

General Principles of Arithmetic (new plan):

For strings, all arithmetic is stack based. That is, there is an
abstract stack of strings from which operations pull their operands
and push their results. This is somewhat like FORTH (or an HP calculator
RPN notation) and spares the need to keep register addresses in
operands. I may find this leads to a more compact form of instruction
code, and may lead to more efficient operators overall, and in
particular I may find improved efficiency overall; so after the
experience of implementing it for strings, I'll want to change other
types around to using this method as well. Keep this in mind whenever
considering adding new instructions to vvp.

Flags
-----

There are up to 16 bits in each thread that are available for
flags. These are used as destinations for operations that return
boolean values, for example comparisons. They are also used as inputs
for test and branch opcodes.

* %abs/wr

This instruction calculates the absolute value of a real value. It uses
the fabs() function in the run-time to do the work, and manipulates
the top of the real-value stack.

* %add
* %addi <vala>, <valb>, <wid>

This opcode pops and adds two vec4 values from the vec4 stack, adds
them, and pushes the result back to the stack. The input values must
have the same size, and the pushed result will have the same width.

The %addi variant takes one operand from the stack, the other is an
immediate value (See %pushi/vec4).

See also the %sub instruction.

* %add/wr

This is the real valued version of the %add instruction. The arguments
are popped from the stack, right operand then left, and the result
pushed in place

See also the %sub/wr instruction.

* %alloc <scope-label>

This instruction allocates the storage for a new instance of an
automatically allocated scope.

* %and

Perform the bitwise AND of the two vectors popped from the vec4 stack,
and push the result. Each bit is calculated independent of other
bits. AND means the following:

	0 and ? --> 0
	? and 0 --> 0
	1 and 1 --> 1
	otherwise   x

The input vectors must be the same width, and the output vector will
be the width of the input.

* %and/r

Pop the top value from the vec4 stack, perform a reduction &, then
return the single-bit result.

* %assign/ar <array-label>, <delay>
* %assign/ar/d <array-label>, <delayx>
* %assign/ar/e <array-label>

The %assign/ar instruction assigns a real value to a word in the
labeled real array. The <delay> is the delay in simulation time to
the assignment (0 for non-blocking assignment) and the value is popped
from the real value stack.

The memory word address is read from index register 3. The address is
in canonical form.

The %assign/ar/d variation reads the delay from an integer register that
is given by the <delayx> value. This should not be 3 or the <bit> index,
of course, since these registers contain the word address and the value.

The %assign/ar/e variation uses the information in the thread
event control registers to determine when to perform the assign.
%evctl is used to set the event control information.

* %assign/v0 <var-label>, <delay>, <bit> (XXXX Old description)
* %assign/v0/d <var-label>, <delayx>, <bit> (XXXX Old description)
* %assign/v0/e <var-label>, <bit> (XXXX Old description)

The %assign/v0 instruction is a vector version of non-blocking
assignment. The <delay> is the number of clock ticks in the future
where the assignment should be schedule, and the <bit> is the base of
the vector to be assigned to the destination. The vector width is in
index register 0.

The %assign/v0/d variation gets the delay instead from an integer
register that is given by the <delayx> value. This should not be 0, of
course, because integer 0 is taken with the vector width.

The %assign/v0/e variation uses the information in the thread
event control registers to determine when to perform the assign.
%evctl is used to set the event control information.

The <var-label> references a .var object that can receive non-blocking
assignments. For blocking assignments, see %set/v.

* %assign/vec4 <var-label>, <delay>
* %assign/vec4/d <var-label>, <delayx>
* %assign/vec4/e <var-label>

The %assign/vec4 instruction is a vec4 version of non-blocking
assignment. The <delay> is the number of clock ticks in the future
where the assignment should schedule, and the value to assign is
pulled from the vec4 stack.

The %assign/vec4/d instruction is the same, but gets its delay value
from the index register <delayx> instead.

* %assign/vec4/a/d <var-label>, <off-index>, <delay-index>
* %assign/vec4/a/e <var-label>, <off-index>

This instruction implements delayed assignment to an array word. The
value is popped from the vec4 stack; the width is taken from the
popped value. The <off-index> index register contains the canonical
offset into the memory word for a part select, and the <delay-index>
index register contains the delay for the assignment. Index register 3
contains the word address.

The <off-index> and <delay-index> index registers can be 0, which
means a zero value instead of the contents of index register 0.

If flag bit 4 is set, then the value will be popped from the stack,
but it will not be assigned. This handles the case that the index
values is undefined.

* %assign/vec4/off/d <var-label>, <off-index>, <delay-index>

This is for writing parts to the target variable. The <var-label> is
the variable to write, as usual. The <off-index> selects an index
register that holds the offset into the target variable, and the
<delay-index> selects the index register that contains the delay. The
offset is in canonical bits. The width that is written is taken from
the width of the value on the stack.

The actual assignment is suppressed if flags-4 is 1. This is so that
calculations of offset can set the flag on errors.

* %assign/wr <vpi-label>, <delay>
* %assign/wr/d <vpi-label>, <delayx>
* %assign/wr/e <vpi-label>

This instruction provides a non-blocking assign of the real value
given in <index> to the real object addressed by the <vpi-label>
label after the given <delay>. The real value is popped from the stack.

The %assign/wr/d variation gets the delay from integer register
<delayx>.

The %assign/wr/e variation uses the information in the thread
event control registers to determine when to perform the assign.
%evctl is used to set the event control information.

* %blend

This instruction blends the bits of two vectors into a result in a
manner line the expressions ('bx ? <a> : <b>). The two source vectors
are popped from the vec4 stack (and must have the same width) and the
result pushed in their place. The truth table for each bit is:

	1  1 --> 1
	0  0 --> 0
	z  z --> z
	x  x --> x
	.... --> x

In other words, if the bits are identical, then take that
value. Otherwise, the value is x.

* %blend/wr

This instruction blends real values for the ternary operator. If the
values match return that otherwise return 0.0. Two values are popped
from the stack, one is pushed back.

* %breakpoint

This instruction unconditionally breaks the simulator into the
interactive debugger. The idea is to stop the simulator here and give
the user a chance to display the state of the simulation using
debugger commands.

This may not work on all platforms. If run-time debugging is compiled
out, then this function is a no-op.

* %callf/obj <code-label>, <scope-label>
* %callf/real <code-label>, <scope-label>
* %callf/str <code-label>, <scope-label>
* %callf/vec4 <code-label>, <scope-label>
* %callf/void <code-label>, <scope-label>

More directly implement function calling. This subsumes the %fork and
%join of the more general task and block calling, but is optimized for
functions, which are threads of a special, constrained sort.

The different variants reflect the different return types for the
called function. For example, if the function returns a string, the
%callf/str opcode is used, and will push the string return value into
the caller's string stack. The %callf/void function is special in that
is pushes no value onto any stack.

* %cassign/vec4 <var-label>
* %cassign/vec4/off <var-label>, <off-index>

Perform a continuous assign of a constant value to the target
variable. This is similar to %set, but it uses the cassign port
(port-1) of the signal functor instead of the normal assign, so the
signal responds differently. See "VARIABLE STATEMENTS" in the
README.txt file.

The %cassign/vec4/off instruction will check the flags[4] flag, and if
it is 1, it will suppress the assignment. This is so that failed index
calculations can report the failure by setting the flag.

* %cassign/wr <var-label>

Perform a continuous assign of a constant real value to the target
variable. See %cassign/v above. The value is popped from the real
value stack.

* %cast2

Pop a value from the vec4 stack, convert it using Verilog rules to a
vector2 (binary) value, and push the result.

* %cast/vec2/dar <wid>

Pop a dynamic array value from the object stack, convert it to a 2-state
vector that is <wid> bits wide, and push the result to the vec4 stack.
If the dynamic array does not fit exactly in <wid> bits, print an error
message and stop the simulation.

* %cast/vec4/dar <wid>

Pop a dynamic array value from the object stack, convert it to a 4-state
vector that is <wid> bits wide, and push the result to the vec4 stack.
If the dynamic array does not fit exactly in <wid> bits, print an error
message and stop the simulation.

* %cast/vec4/str <wid>

Pop a value from the string stack, convert it to a vector that is <wid>
bits wide, and push the result to the vec4 stack. If the string does not
fit exactly in <wid> bits, print an error message and stop the simulation.

* %cmp/s
* %cmp/u
* %cmp/e
* %cmp/ne
* %cmpi/s <vala>, <valb>, <wid>
* %cmpi/u <vala>, <valb>, <wid>
* %cmpi/e <vala>, <valb>, <wid>
* %cmpi/ne <vala>, <valb>, <wid>

These instructions perform a generic comparison of two vectors of
equal size. Two values are pulled from the top of the stack, and not
replaced. The results are written into flag bits 4,5,6. The
expressions (a<b), (a==b) and (a===b) are calculated, with (b) popped
from the stack first, then (a).

The results of the comparison go into flags 4, 5, 6 and 7:

	4: eq  (equal)
	5: lt  (less than)
	6: eeq (case equal)

The eeq bit is set to 1 if all the bits in the vectors are exactly the
same, or 0 otherwise. The eq bit is true if the values are logically
the same. That is, x and z are considered equal. In other words the eq
bit is the same as `==` and the eeq bit `===`.

The lt bit is 1 if the left vector is less than the right vector, or 0
if greater than or equal to the right vector. It is the equivalent of
the Verilog < operator. Combinations of these three bits can be used
to implement all the Verilog comparison operators.

The %cmp/u and %cmp/s differ only in the handling of the lt bit. The
%cmp/u does an unsigned compare, whereas the %cmp/s does a signed
compare. In either case, if either operand contains x or z, then lt
bit gets the x value.

The %cmp/e and %cmpi/e variants are the same, but they do not bother
to calculate the lt flag. These are faster if the lt flag is not needed.

The %cmp/ne and %cmpi/ne variants are the same as the %cmp/e and
%cmpi/e variants, but the 4 and 6 flags are inverted in order to
eliminate the need for a %flag_inv instruction to implement != and !==
operations.

* %cmp/we
* %cmp/wne

These instructions perform a wild comparison of two vectors of equal
size. Two values are pulled from the top of the stack, and not replaced.
The results are written into flag bit 4. The comparisons work like eq/ne
except an x/z bit in the r-value will match any l-value bit.

The %cmp/wne variant is the same as %cmp/we, but the 4 flag is inverted
in order to eliminate the need for a %flag_inv instruction to implement
the !=? operator.

* %cmp/wr

Compare real values for equality and less-then. This opcode pops to
values from the real-value stack and writes the comparison result to
bits 4/5. The expressions (a < b) and (a==b) are calculated, with (b)
popped from the stack first, then (a).

* %cmp/z
* %cmp/x

These instructions are for implementing the casez and casex
comparisons. These work similar to the %cmp/u instructions, except
only an eq bit is calculated. These comparisons both treat z values in
the left or right operand as don't care positions. The %cmp/x
instruction will also treat x values in either operand as don't care.

Only bit 4 is set by these instructions.

* %cmp/str

This instruction pops the top two strings from the string stack and
compares them. The results of the comparison go into bits 4 and 5:

	4: eq  (equal)
	5: lt  (less than)

For the purposes of calculating the lt bit, the top string is the
right operand and the string underneath is the left operand. This
instruction removes two strings from the stack.

* %concat/str
* %concati/str <string>

Pop the top string, and concatenate it to the new top string. Or think
of it as passing the tail, then the head, concatenating them, and
pushing the result. The stack starts with two strings in the stack,
and ends with one string in the stack.

The %concati/str form pops only one value from the stack. The right
part comes from the immediate value.

* %concat/vec4
* %concati/vec4 <vala>, <valb>, <wid>

Pop two vec4 vectors, concatenate them, and push the combined
result. The top of the vec4 stack is the LSB of the result, and the
next in this stack is the MSB bits of the result.

The %concati/vec4 form takes an immediate value and appends it (lsb)
to the value on the top of the stack. See the %pushi/vec4 instruction
for how to describe the immediate value.

* %cvt/sr <index>
* %cvt/ur <bit-l>

Pop a word from the real-value stack, convert it to a signed or
unsigned integer, and write it to the <index> index
register. Precision may be lost in the conversion.

* %cvt/rv
* %cvt/rv/s

The %cvt/rv instruction pops a value from the thread vec4 stack and
converts it to a real word. Push the result onto the real value
stack. Precision may be lost in the conversion.

The %cvt/rv/s instruction is the same as %cvt/rv, but treats the thread
vector as a signed value.

* %cvt/vr <wid>

The %cvt/vr opcode converts a real word from the stack to a vec4 that
is <wid> wide. Non-integer precision is lost in the conversion, and
the real value is popped from the stack. The result is pushed to the
vec4 stack.

* %deassign <var-label>, <base>, <width>

Deactivate and disconnect a procedural continuous assignment to a
variable. The <var-label> identifies the affected variable.

The <base> and <width> are used to determine what part of the signal
will be deactivated. For a full deactivation the <base> is 0 and
<width> is the entire signal width.

* %deassign/wr <var-label>

The same as %deassign above except this is used for real variables.

* %debug/thr

These opcodes are aids for debugging the vvp engine. The vvp code
generator should not generate these, and they should not alter code
flow, data contents, etc.

* %delay <low>, <high>

This opcode pauses the thread, and causes it to be rescheduled for a
time in the future. The amount is the number of the ticks in the
future to reschedule, and is >= 0. If the %delay is zero, then the
thread yields the processor for another thread, but will be resumed in
the current time step.

The delay amount is given as 2 32bit numbers, so that 64bit times may
be represented.

* %delayx <idx>

This is similar to the %delay opcode, except that the parameter
selects an index register, which contains the actual delay. This
supports run-time calculated delays.

* %delete/obj <var-label>

Arrange for the dynamic object at the target label to be deleted.
This has no effect on the object or string stack. Note that this is
the same as:

   %null ;
   %store/obj <var-label>

but that idiom is expected to be common enough that it warrants an
optimized shorthand.

* %disable <scope-label>

This instruction terminates threads that are part of a specific
scope. The label identifies the scope in question, and the threads are
the threads that are currently within that scope.

* %disable/flow <scope-label>

This instruction is similar to `%disable` except that it will only disable a
single thread of the specified scope. The disabled thread will be the thread
closest to the current thread in the thread hierarchy. This can either be thread
itself or one of its parents.

It is used to implement flow control statements called from within a thread that
only affect the thread or its parents. E.g. SystemVerilog `return`, `continue`
or `break`.

* %disable/fork

This instruction terminates all the detached children for the current
thread. There should not be any non-detached children.


* %div <bit-l>, <bit-r>, <wid>
* %div/s <bit-l>, <bit-r>, <wid>

This instruction arithmetically divides the <bit-l> vector by the
<bit-r> vector, and leaves the result in the <bit-l> vector. IF any of
the bits in either vector are x or z, the entire result is x.

The %div/s instruction is the same as %div, but does signed division.


* %div/wr

This opcode divides the left operand by the right operand. If the
right operand is 0, then the result is NaN.


* %dup/real
* %dup/vec4
* %dup/obj

These opcodes duplicate the value on the top of the stack for the
corresponding type.

* %evctl <functor-label> <idx>
* %evctl/c
* %evctl/s <functor-label> <idx>
* %evctl/i <functor-label> <value>

These instructions are used to put event and repetition information
into the thread event control registers. These values are then used
by the %assign/e instructions to do not blocking event control. The
<functor-label> is the event to trigger on and the <idx> is an index
register to read the repetition count from (signed or unsigned).
%evctl/i sets the repetition to an immediate unsigned value.

%evctl/c clears the event control information. This is needed if a
%assign/e may be skipped since the %assign/e statements clear the
event control information and the other %evctl statements assert
that this information has been cleared. You can get an assert if
this information is not managed correctly.

* %event <functor-label>
* %event/nb <functor-label>

This instruction is used to send a pulse to an event object. The
<functor-label> is an event variable. This instruction simply writes
an arbitrary value to the event to trigger the event.

* %file_line <file> <line> <description>

This command emits the provided file and line information along with
the description when it is executed. The output is sent to stderr and
the format of the output is:

   <file>:<line>: <description>

<file> is the unsigned numeric file index.
<line> is the unsigned line number.
<description> is a string, if string is 0 then the following default
message is used: "Procedural tracing.".

* %flag_inv <flag>

This instruct inverts a flag bit.

* %flag_mov <flag1>, <flag2>

This instruction copies the flag bit from <flag2> to <flag1>.

* %flag_or <flag1>, <flag2>

This instruction calculates the Verilog OR of the flag bits in <flag1>
and <flag2>, and leaves the result in <flag1>.

* %flag_set/imm <flag>, <value>

This instruction sets an immediate value into a flag bit. This is a
single bit, and the value is 0==0, 1==1, 2==z, 3==x.

* %flag_get/vec4 <flag>
* %flag_set/vec4 <flag>

These instructions provide a means for accessing flag bits. The
%flag_get/vec4 loads the numbered flag as a vec4 on top of the vec4
stack, and the %flag_set/vec4 pops the top of the vec4 stack and
writes the LSB to the selected flag.

* %force/vec4 <label>
* %force/vec4/off <label>, <off>
* %force/vec4/off/d <label>, <off>, <delay>

Perform a "force" assign of a values to the target variable. The value
to be forced is popped from the vec4 stack and forced to the target
variable. The /off variant forces a part of a vector. The width of the
part comes from the width of the popped value, and the <off> is an
index register that contains the canonical offset where the value
gets written. The <delay> is an index register that contains the
delay.

The %force/vec4/off instructions will test the value of flags[4], and if
it is 1, will suppress the actual assignment. This is intended to help
with detection of invalid index expressions.

* %force/wr <var-label>

Force a constant real value to the target variable. See %force/v
above. The value is popped from the real value stack.

* %fork <code-label>, <scope-label>

This instruction is similar to %jmp, except that it creates a new
thread to start executing at the specified address. The new thread is
created and pushed onto the child stack.  It is also marked runnable,
but is not necessarily started until the current thread yields.

The %fork instruction has no effect other than to push a child thread.

See also %join.

* %free <scope-label>

This instruction de-allocates the storage for a previously allocated
instance of as automatically allocated scope.


* %inv

Perform a bitwise invert of the vector on top of the vec4 stack. The result
replaces the input. Invert means the following, independently, for each
bit:

	0  --> 1
	1  --> 0
	x  --> x
	z  --> x


* %ix/vec4 <idx>
* %ix/vec4/s <idx>

This instruction loads a vec4 value from the vec4 stack, into the
index register <idx>. The value is popped from the vec4 stack and
written to the index register.

The %ix/vec4 instruction converts the 4-value bits into a binary
number, without sign extension. If any of the bits of the vector is x
or z, then the index register gets the value 0. The %ix/vec4/s
instruction is the same, except that it assumes the source vector is
sign extended to fit the index register.

The instruction also writes into flag 4 a 1 if any of the bits of the
input vector are x or z. This is a flag that the 0 value written into
the index register is really the result of calculating from unknown
bits. It writes an X into flag 4 if the vec4 value overflows the index
register.

	4: unknown value or overflow
	5: (reserved)
	6: (reserved)

* %ix/getv <idx>, <functor-label>
* %ix/getv/s <idx>, <functor-label>

These instructions are like the %ix/vec4 instructions, except that they
read directly from a functor label instead of from thread bits. They set
flag 4 just like %ix/get (overflow is not currently checked by ix/getv/s).

* %ix/load <idx>, <low>, <high>

This instruction loads an immediate value into the addressed index
register. The index register holds 64 bit numeric values, so <low>
and <high> are used to separate the value in two 32 bit chunks.
The idx value selects the index register. This is different from
%ix/get, which loads the index register from a value in the thread bit
vector. The values are unsigned decimal values and are combined as
<high> << 32 | <low> to produce the final value.


* %ix/add <idx>, <low>, <high>
* %ix/sub <idx>, <low>, <high>
* %ix/mul <idx>, <low>, <high>

These instructions add, subtract, or multiply the selected index
register by the immediate value. The 64 bit immediate value is built
from the two 32 bit chunks <low> and <high> (see %ix/load above).
The <idx> value selects the index register.

* %ix/mov <dst>, <src>

This instruction simply sets the index register <dst> to the value of
the index register <src>.

* %jmp <code-label>

The %jmp instruction performs an unconditional branch to a given
location. The parameter is the label of the destination instruction.

* %jmp/[01xz] <code-label>, <flag>

This is a conditional version of the %jmp instruction. In this case,
a flag bit (addressed by <bit>) is tested. If it is one of the
values in the part after the /, the jump is taken. For example:

	%jmp/xz T_label, 8;

will jump to T_label if bit 8 is x or z.

* %join

This is the partner to %fork. This instruction causes the thread to
wait for the top thread in the child stack to terminate, then
continues. It has no effect in the current thread other than to wait
until the top child is cleared.

It is an error to execute %join if there are no children in the child
stack. Every %join in the thread must have a matching %fork that
spawned off a child thread.

If the matching child instruction is still running, a %join suspends
the calling thread until the child ends. If the child is already
ended, then the %join does not block or yield the thread.

* %join/detach <n>

This is also a partner to %fork. This instruction causes the thread
to detach <n> threads from the current thread. The <n> should be ALL
the children, and none of those children may be automatic. This
instruction is used to implement join_none and join_any from the
Verilog source.

* %load/obj <var-label>

This instruction loads an object handle and pushes it to the top of
the object handle stack.

See also %store/obj.

* %load/real <vpi-label>
* %load/dar/r <functor-label>

The %load/real instruction reads a real value from the vpi-like object
and pushes it to the top of the real value stack.

The %load/dar/r loads the real word from the darray and pushes it onto
the stack.

* %load/str <var-label>
* %load/stra <array-label>, <index>
* %load/dar/str <var-label>

The %load/str instruction gets the string from the string variable and
pushes in to the string stack. (See also %store/str)

The %load/dar/str is similar, but the variable is a dynamic array of
strings, and there is an index value in index register 3.
(See also %store/dar/str)


* %load/v <bit>, <functor-label>, <wid> (XXXX Old implementation)

This instruction loads a vector value from the given functor node into
the specified thread register bit. The functor-label can refer to a
.net, a .var or a .functor with a vector output. The entire vector,
from the least significant up to <wid> bits, is loaded starting at
thread bit <bit>. It is an OK for the width to not match the vector
width at the functor. If the <wid> is less than the width at the
functor, then the most significant bits are dropped. If the <wid> is
more than the width at the functor, the value is padded with X bits.

* %load/vec4 <var-label>

This instruction loads a vector value from the given functor node and
pushes it onto the vec4 stack. See also the %store/vec4 instruction.

* %load/vec4a <arr-label>, <addr-index>

This instruction loads a vec4 value from the array and pushes the
value onto the stack. The <addr-index> is the index register that
holds the canonical array index.

The load checks flag bit 4. If it is 1, then the load it cancelled and
replaced with a load of all X bits. See %ix/vec4.

* %load/ar <array-label>, <index>

The %load/ar instruction reads a real value from an array. The <index>
is the index register that contains the canonical word address into
the array.

* %loadi/wr <bit>, <mant>, <exp>

This opcode loads an immediate value, floating point, into the word
register selected by <bit>. The mantissa is an unsigned integer value,
up to 32 bits, that multiplied by 2**(<exp>-0x1000) to make a real
value. The sign bit is OR-ed into the <exp> value at bit 0x4000, and
is removed from the <exp> before calculating the real value.

If <exp>==0x3fff and <mant> == 0, the value is +inf.
If <exp>==0x7fff and <mant> == 0, the value is -inf.
If <exp>==0x3fff and <mant> != 0, the value is NaN.

* %max/wr
* %min/wr

This instruction pops the top two values from the real stack and
pushes back the max(min) value. Avoid returning NaN by selecting the
other if either is NaN.

* %mod
* %mod/s

This instruction calculates the modulus %r of the left operand, and
replaces the left operand with the result. The left and right vectors
are popped from the vec4 stack and have identical width. The result is
pushed onto the vec4 stack.

The /s form does signed %.

* %mod/wr

This opcode is the real-valued modulus of the two real values.

* %mul
* %muli <vala>, <valb>, <wid>

This instruction multiplies the left vector by the right vector, the
vectors pare popped from the vec4 stack and have the same width. If
any of the bits of either vector are x or z, the result is
x. Otherwise, the result is the arithmetic product. In any case, the
result is pushed back on the vec4 stack.


* %mul/wr

This opcode multiplies two real words together.

* %nand

Perform the bitwise NAND of two vec4 vectors, and push the result. Each
bit is calculated independent of other bits. NAND means the following:

	0 and ? --> 1
	? and 0 --> 1
	1 and 1 --> 0
	otherwise   x


* %new/cobj <label>

Create a new class object. The <label> is the VPI label for a class
type definition.

* %new/darray <idx>, "<type>"

Create a new array (of int objects) with a size. the <idx> is the
address of an index variable that contains the computed array size to
use. The <type> is a string that expresses the type of the elements of
the array. See also %delete/obj

The supported types are:

         "b<N>"     - unsigned bool <N>-bits
         "sb<N>"    - signed bool <N>-bits
	 "r"        - real
	 "S"        - SystemVerilog string

* %nor

Perform the bitwise nor of vec4 vectors, and push the result. Each bit
in the source vectors is combined to make a result bit according to the
truth table.

	1 nor ? --> 0
	? nor 1 --> 0
	0 nor 0 --> 1
	otherwise  x


* %nor/r <dst>, <src>, <wid> (XXXX Old definition)

The %nor/r instruction is a reduction nor. That is, the <src> is a
vector with width, but the result is a single bit. The <src> vector is
not affected by the operation unless the <dst> bit is within the
vector. The result is calculated before the <dst> bit is written, so
it is valid to place the <dst> within the <src>.

The actual operation performed is the inverted or of all the bits in
the vector.

* %nor/r

The %nor/r instruction is a reduction nor. That is, a vec4 value is
popped from the vec4 stack, the bits of the vector are or'ed together
to a signal bit, that bit is inverted and the resulting 1-bit vector
pushed back to the vec4 stack. See also the "%or" instruction.

* %null

Push a null object and push it to the object stack. The null object
can be used with any class or darray object, so it is not typed.

* %or

Perform the bitwise or of two vectors. Pop two values from the vec4
stack to get the input arguments. Each bit in the result is combined
with the corresponding bit in the input arguments, according to the
truth table:

	1 or ? --> 1
	? or 1 --> 1
	0 or 0 --> 0
	otherwise  x

The results is then pushed onto the vec4 stack. The inputs and the
output are all the same width.

* %or/r

This is a reduction version of the %or opcode. Pop a single value from
the vec4 stack, perform the reduction or and return the result to the stack.

* %pad <dst>, <src>, <wid> (XXXX Old version)

This instruction replicates a single bit in register space into a
destination vector in register space. The destination may overlap
the source bit. The <dst> may not be 0-3. This is useful for zero
or sign extending a vector.

* %pad/s <wid>
* %pad/u <wid>

These instruction change the size of the top item in the vec4
stack. If this item is larger then this, it is truncated. If smaller,
then extended. The /s variant sign extends, the /u variant unsigned
extends.

* %part/s <wid>
* %part/u <wid>

This instruction implements a part select. It pops from the top of the
vec4 the base value, then it pops the base to select from. The width
is the fixed number <wid>. The result is pushed back to the stack.

* %pop/str <num>
* %pop/real <num>
* %pop/obj <num>, <skip>
* %pop/vec4 <num>

Pop <num> items from the string/real/object/vec4 stack. This is the
opposite of the %pushX/str opcode which pushes a string to the
stack. The %pop/str is not normally needed because the %store/str
includes an implicit pop, but sometimes it is necessary to pop
explicitly.

The <skip> is the number of top positions on the stack to keep,
before starting to pop. This allows for popping positions other than
the top of the stack.

* %pow
* %pow/s

The %pow opcode pops the "B" value from the stack, then pops the "A"
value from the stack, then pushes A**B back onto the stack. Of there
are any X or Z bits in A or B, then an X value is pushed onto the
stack instead of a calculated values.

The %pow/s opcode does the same for signed values.

* %pow/wr

This opcode raises the left operand by the right operand, and pushes
the result.

* %prop/v <pid>
* %prop/obj <pid>, <idx>
* %prop/r <pid>
* %prop/str <pid>

Read a vector (/v) or real value (/r) or string (/str) or object from
the property number <pid> of the class object on the top of the
object stack. Push the resulting value to the appropriate stack. The
class object that is the source is NOT popped from the object stack.

The <idx> is the address of an index variable that selects the word of
an arrayed property. If the <idx> value is 0, then use index value
zero instead of reading index register zero. Use this form for
non-arrayed properties.

* %pushi/real <mant>, <exp>

This opcode loads an immediate value, floating point, into the real
value stack. The mantissa is an unsigned integer value, up to 32 bits,
that multiplied by 2**(<exp>-0x1000) to make a real value. The sign
bit is OR-ed into the <exp> value at bit 0x4000, and is removed from
the <exp> before calculating the real value.

If <exp>==0x3fff and <mant> == 0, the value is +inf.
If <exp>==0x7fff and <mant> == 0, the value is -inf.
If <exp>==0x3fff and <mant> != 0, the value is NaN.

* %pushi/str <text>

Push a literal string to the string stack.

* %pushi/vec4 <vala>, <valb>, <wid>

This opcode loads an immediate value, vector4, into the vector
stack. The <vala> is the boolean value bits, and the <valb> bits are
modifiers to support z and x values. The a/b encodings for the 4
possible logic values are:

   a b  val
   0 0   0
   1 0   1
   1 1   x
   0 1   z

This opcode is limited to 32bit numbers.

* %pushv/str

Convert a vector to a string and push the string to the string
stack. The single argument is popped from the vec4 stack and the
result pushed to the string stack.

* %putc/str/v <functor-label>, <muxr>

Pop a vector byte from the thread vec4 stack and write it to a
character of the string variable at <functor-label>. This is
basically an implementation of <string>.putc(<muxr>, <val>) where
<val> is the 8bit vector popped from the stack.

* %qpop/b/real <functor-label>
* %qpop/f/real <functor-label>
* %qpop/b/str <functor-label>
* %qpop/f/str <functor-label>
* %qpop/b/v <functor-label>
* %qpop/f/v <functor-label>

Pop values from a dynamic queue object.

* %release/net <functor-label>, <base>, <width>
* %release/reg <functor-label>, <base>, <width>

Release the force on the signal that is represented by the functor
<functor-label>.  The force was previously activated with a %force/v
statement.  If no force was active on this functor the statement does
nothing. The %release/net sends to the labeled functor the release
command with net semantics: the unforced value is propagated to the
output of the signal after the release is complete. The %release/reg
sends the release command with reg semantics: the signal holds its
forced value until another value propagates through.

The <base> and <width> are used to determine what part of the signal
will be released. For a full release the <base> is 0 and <width> is
the entire signal width.

* %release/wr <functor-label>, <type>

Release the force on the real signal that is represented by the functor
<functor-label>.  The force was previously activated with a %force/wr
statement. The <type> is 0 for nets and 1 for registers. See the other
%release commands above.

* %replicate <count>

Pop the vec4 value, replicate it <count> times, then push the
result. In other words, push the concatenation of <count> copies.
See also the %concat instruction.

* %ret/obj <index>
* %ret/real <index>
* %ret/str <index>
* %ret/vec4 <index>, <offset>, <wid>

Write a value to the indexed function argument. The value is popped
from the appropriate stack and written into the argument. The return
value of a function is the first argument of the appropriate type, so
for example to store the return value for a real function, use
"%ret/real 0;". It is up to the function caller to set up the argument
references.

The %ret/vec4 opcode works very much like the %store/vec4 opcode. The
<off> and <wid> operands are the offset and width of the subvector of
the destination value that is written by the value popped from the
vec4 stack. Off the <offset> is zero, then the literal offset is
zero. If the <offset> is non-zero, then it selects an index register
that contains the actual offset. In this case, flag-4 is tested, and
if not 1, the assign is suppressed.

* %retload/vec4 <index>
* %retload/real <index>
* %retload/str <index>

Read a value from the indexed function argument. The value is read
from the argument and pushed to the appropriate stack.

* %set/dar/obj/real <index>
* %set/dar/obj/str <index>
* %set/dar/obj/vec4 <index>

The "%set/dar/obj/real" opcode sets the top value from the real-value
stack to the index. This does NOT pop the real value off the
stack. The intent is that this value may be written to a bunch of
values.

The "%set/dar/obj/str" opcode does the same but for string values and
uses the string stack, and the "%set/dar/obj/vec4" for vec4 values and
the vector stack.

* %set/v <var-label>, <bit>, <wid> (XXXX Old definition)

This sets a vector to a variable, and is used to implement blocking
assignments. The <var-label> identifies the variable to receive the
new value. Once the set completes, the value is immediately available
to be read out of the variable. The <bit> is the address of the thread
register that contains the LSB of the vector, and the <wid> is the
size of the vector. The width must exactly match the width of the
signal.

* %set/qb <var-label>, <bit>, <wid>
* %set/qf <var-label>, <bit>, <wid>

This sets the vector value into the back (qb) or front (qf) of a queue
variable.

* %shiftl <idx>
* %shiftr <idx>
* %shiftr/s <idx>

These instructions shift the top value in the vec4 stack left (towards
MSB) or right, possibly signed. The <idx> is the address of the index
register that contains the amount to shift.

The instruction also checks flag bit 4. If it is true, the result is
replaced with X instead of a shifted result. This is intended to
detect that the index contents were not valid.

* %split/vec4 <wid>

Pull the top vec4 vector from the stack and split it into two
parts. Split off <wid> bits from the LSB, then push the remaining bits
of the original (the MSB) back to the stack. Then push the split off
LSB vector.

The <wid> must be less than the width of the original, unsplit vector.

* %store/obj <var-label>

This pops the top of the object stack and writes it to the object
variable given by the label.

See also %load/obj.

* %store/prop/obj <pid>, <idx>
* %store/prop/r <pid>
* %store/prop/str <pid>
* %store/prop/v <pid>, <wid>

The %store/prop/r pops a real value from the real stack and stores it
into the the property number <pid> of a cobject in the top of the
object stack. The cobject is NOT popped.

The %store/prop/obj pops an object from the top of the object stack,
then writes it to the property number <pid> of the cobject now on
top of the object stack. The cobject is NOT popped. The <idx> argument
is the index register to select an element. If the property is an
array, this selects the element. If <idx> is 0, then use the value 0
instead of index register zero. Use 0 for non-array properties.

The %store/prop/v pops a vector from the vec4 stack and stores it into
the property <pid> of the cobject in the top of the object stack. The
vector is truncated to <wid> bits, and the cobject is NOT popped.

* %store/real <var-label>
* %store/reala <var-label>, <index>

This pops the top of the real variable stack and write it to the
object variable given by the label.

The reala version is similar, but writes to a real array using the
index in the index register <index>

* %store/str <var-label>
* %store/stra <array-label>, <index>
* %store/dar/r <var-label>
* %store/dar/str <var-label>
* %store/dar/vec4 <var-label>
* %store/qf/r <var-label>
* %store/qf/str <var-label>
* %store/qf/v <var-label>, <wid>
* %store/qb/r <var-label>
* %store/qb/str <var-label>
* %store/qf/v <var-label>, <wid>

The %store/str instruction pops the top of the string stack and writes
it to the string variable.

The %store/stra targets an array.

The %store/dar/str is similar, but the target is a dynamic array of
string string. The index is taken from signed index register 3.

* %store/vec4 <var-label>, <offset>, <wid>
* %store/vec4a <var-label>, <addr>, <offset>

Store a logic vector into the variable. The value (and its width) is
popped off the top of the stack and written to the variable. The value
is then optionally truncated to <wid> bits and assigned to the
variable. It is an error for the value to be fewer then <wid>
bits. The <offset> is the index register that contains a part offset
for writing into a part of the variable. If the <offset> is 0, then
use the literal value 0 instead of getting an offset from index
register 0.

The %store/vec4a is similar, but the target is an array of vec4, the
<addr> is an index register that contains the canonical address, and
the <offset> is an index register that contains the vector part
offset.

Both index registers can be 0, to mean a zero value instead of a zero
register.

The %store/vec4a will check flag bit 4, and if it is true, it will
suppress the actual assignment. This is so that the context can
indicate that the address is invalid.

The %store/vec4 will check flag bit 4, only if the <offset> is a
non-zero index register. A 0 index is a fixed constant and cannot
fail.

NOTE: The <wid> is not necessary, and should be removed.

The %store/qf/* and %store/qb/* instructions are queue manipulations,
which implement the push_back (qb) and push_front (qf)
functions. These only apply to queue object, and are distinct from the
dar versions because the begin/front don't exist, by definition.

* %sub

This instruction subtracts vec4 values. The right value is popped from
the vec4 stack, then the left value is popped. The right is subtracted
from the left, and the result pushed.

See also the %add instruction.

* %subi <vala>, <valb>, <wid>

This instruction pops a value "A" from the vec4 stack, generates a
values "B" from the immediate argument, and pushes A-B.

See also the %addi instruction.

* %sub/wr

This instruction operates on real values in word registers. The right
value is popped, the left value is popped, the right is subtracted
from the left, and the result pushed.

* %substr <start>, <end>

This instruction takes the substring of the top string in the string
stack. This implements the SystemVerilog style substring. The string
stack is popped and replaced with the result.

* %substr/vec4 <sel>, <wid>

This instruction extracts the substring of the top string in the
string stack and pushes the result to the vec4 stack. The <sel> is the
index register that holds the select base, and the <wid> is the width,
in bits, of the result. Note that <wid> must be a multiple of 8.

The string value is NOT popped.


* %test_nul <var-label>
* %test_nul/obj
* %test_nul/prop <pid>, <idx>

This instruction tests the contents of the addressed variable to see
if it is null. If it is, set flag bit 4 to 1. Otherwise, set flag bit
4 to 0.

The %test_null/obj tests the object on the top of the stack, instead
of any variable. The value in the stack is NOT popped.

The %test_nul/prop instruction tests an object property for nul. The
object with the property is peeked from the top of the object stack,
and the <idx> is the array index if the property is an array of objects.

This is intended to implement the SystemVerilog expression
(<var>==null), where <var> is a class variable.

* %vpi_call <name> [, ...] {<vec4> <real> <str>}

This instruction makes a call to a system task that was declared using
VPI. The operands are compiled down to a vpiHandle for the call. The
instruction contains only the vpiHandle for the call. See the vpi.txt
file for more on system task/function calls.

The {...} part is stack information. This tells the run-time how many
stack items the call uses so that it knows how many to pop off the
stack when the call returns.

* %vpi_func <file> <line> <name> [, ...] {<vec4> <real> <str>}
* %vpi_func/r <file> <line> <name> [, ...] {<vec4> <real> <str>}
* %vpi_func/s <file> <line> <name> [, ...] {<vec4> <real> <str>}

This instruction is similar to %vpi_call, except that it is for
calling system functions. The difference here is the return value from
the function call is pushed onto the appropriate stack. The normal
means that the VPI code uses to write the return value causes those
bits to go here.

The {...} part is stack information. This tells the run-time how many
stack items the call uses from each stack so that it knows how many to
pop off the stack when the call returns. The function call will pop
the real and string stacks, and will push any return value.


* %wait <functor-label>

When a thread executes this instruction, it places itself in the
sensitive list for the addressed functor. The functor holds all the
threads that await the functor. When the defined sort of event occurs
on the functor, a thread schedule event is created for all the threads
in its list and the list is cleared.

* %wait/fork

This instruction puts the current thread to sleep until all the detached
children have finished executing. The last detached child is responsible
for restarting the parent when it finishes.

* %xnor

This instruction pops two vectors from the vec4 stack, does a bitwise
exclusive nor (~^) of the vectors, and pushes the result. The truth
table for the xor is:

	0 xnor 0 --> 1
	0 xnor 1 --> 0
	1 xnor 0 --> 0
	1 xnor 1 --> 1
	otherwise    x


* %xor

This instruction pops two vectors from the vec4 stack, does a bitwise
exclusive or (^) of the vectors, and pushes the result. The truth
table for the xor is:

	0 xor 0 --> 0
	0 xor 1 --> 1
	1 xor 0 --> 1
	1 xor 1 --> 0
	otherwise   x

::

    /*
     * Copyright (c) 2001-2023 Stephen Williams (steve@icarus.com)
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
     *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
     */
