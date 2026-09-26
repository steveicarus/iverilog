
IEEE1364 Notes
==============

The IEEE1364 standard is the bible that defines the correctness of the
Icarus Verilog implementation and behavior of the compiled
program. The IEEE1364.1 is also referenced for matters of
synthesis. So the ultimate definition of right and wrong comes from
those documents.

That does not mean that a Verilog implementation is fully
constrained. The standard document allows for implementation specific
behavior that, when properly accounted for, does not effect the
intended semantics of the specified language. It is therefore possible
and common to write programs that produce different results when run
by different Verilog implementations.


Standardization Issues
----------------------

These are some issues where the IEEE1364 left unclear, unspecified or
simply wrong. I'll try to be precise as I can, and reference the
standard as needed. I've made implementation decisions for Icarus
Verilog, and I will make clear what those decisions are and how they
affect the language.

* OBJECTS CAN BE DECLARED ANYWHERE IN THE MODULE

Consider this module::

    module sample1;
        initial foo = 1;
    reg foo;
    wire tmp = bar;
    initial #1 $display("foo = %b, bar = %b", foo, tmp);
    endmodule

Notice that the `reg foo;` declaration is placed after the first
initial statement. It turns out that this is a perfectly legal module
according to the -1995 and -2000 versions of the standard. The
statement `reg foo;` is a module_item_declaration which is in turn a
module_item. The BNF in the appendix of IEEE1364-1995 treats all
module_item statements equally, so no order is imposed.

Furthermore, there is no text (that I can find) elsewhere in the
standard that imposes any ordering restriction. The sorts of
restrictions I would look for are "module_item_declarations must
appear before all other module_items" or "variables must be declared
textually before they are referenced." Such statements simply do not
exist. (Personally, I think it is fine that they don't.)

The closest is the rules for implicit declarations of variables that
are otherwise undeclared. In the above example, `bar` is implicitly
declared and is therefore a wire. However, although `initial foo = 1;`
is written before foo is declared, foo *is* declared within the
module, and declared legally by the BNF of the standard.

Here is another example::

    module sample2;
	initial x.foo = 1;
        test x;
	initial #1 $display("foo = %b", x.foo);
    endmodule

    module test;
        reg foo;
    endmodule;

From this example one can clearly see that foo is once again declared
after its use in behavioral code. One also sees a forward reference of
an entire module. Once again, the standard places no restriction on
the order of module declarations in a source file, so this program is,
according to the standard, perfectly well formed.

Icarus Verilog interprets both of these examples according to "The
Standard As I Understand It." However, commercial tools in general
break down with these programs. In particular, the first example
may generate different errors depending on the tool. The most common
error is to claim that `foo` is declared twice, once (implicitly) as
a wire and once as a reg.

So the question now becomes, "Is the standard broken, or are the tools
limited?" Coverage of the standard seems to vary widely from tool to
tool so it is not clear that the standard really is at fault. It is
clear, however, that somebody goofed somewhere.

My personal opinion is that there is no logical need to require that
all module_item_declarations precede any other module items. I
personally would oppose such a restriction. It may make sense to
require that declarations of variables within a module be preceded by
their use, although even that is not necessary for the implementation
of efficient compilers.

However, the existence hierarchical naming syntax as demonstrated in
sample2 can have implications that affect any declaration order
rules. When reaching into a module with a hierarchical name, the
module being referenced is already completely declared (or not
declared at all, as in sample2) so module_item order is completely
irrelevant. But a "declare before use" rule would infect module
ordering, by requiring that modules that are used be first defined.


* TASK AND FUNCTION PARAMETERS CANNOT HAVE EXPLICIT TYPES

Consider a function negate that wants to take a signed integer value
and return its negative::

	function integer negate;
	    input [15:0] val;
	    negate = -val;
	endfunction

This is not quite right, because the input is implicitly a reg type,
which is unsigned. The result, then, will always be a negative value,
even if a negative val is passed in.

It is possible to fix up this specific example to work properly with
the bit pattern of a 16bit number, but that is not the point. What's
needed is clarification on whether an input can be declared in the
port declaration as well as in the contained block declaration.

As I understand the situation, this should be allowed::

	function integer negate;
	    input [15:0] val;
	    reg signed [15:0] val;
	    negate = -val;
	endfunction

In the -1995 standard, the variable is already implicitly a reg if
declared within a function or task. However, in the -2000 standard
there is now (as in this example) a reason why one might want to
actually declare the type explicitly.

I think that a port *cannot* be declared as an integer or time type
(though the result can) because the range of the port declaration must
match the range of the integer/time declaration, but the range of
integers is unspecified. This, by the way, also applies to module
ports.

With the above in mind, I have decided to *allow* function and task
ports to be declared with types, as long as the types are variable
types, such as reg or integer. Without this, there would be no
portable way to pass integers into functions/tasks. The standard does
not say it is allowed, but it doesn't *disallow* it, and other
commercial tools seem to work similarly.


* ROUNDING OF TIME

When the \`timescale directive is present, the compiler is supposed to
round fractional times (after scaling) to the nearest integer. The
confusing bit here is that it is apparently conventional that if the
\`timescale directive is *not* present, times are rounded towards zero
always.


* VALUE OF X IN PRIMITIVE OUTPUTS

The IEEE1364-1995 standard clearly states in Table 8-1 that the x
symbols is allowed in input columns, but is not allowed in
outputs. Furthermore, none of the examples have an x in the output of
a primitive. Table 8-1 in the IEEE1364-2000 also says the same thing.

However, the BNF clearly states that 0, 1, x and X are valid
output_symbol characters. The standard is self contradictory. So I
take it that x is allowed, as that is what Verilog-XL does.


* REPEAT LOOPS vs. REPEAT EVENT CONTROL

There seems to be ambiguity in how code like this should be parsed::

	repeat (5) @(posedge clk) <statement>;

There are two valid interpretations of this code, from the
IEEE1364-1995 standard. One looks like this::

    procedural_timing_control_statement ::=
          delay_or_event_control  statement_or_null

    delay_or_event_control ::=
          event_control
          | repeat ( expression ) event_control

If this interpretation is used, then the statement <statement> should
be executed after the 5th posedge of clk. However, there is also this
interpretation::

    loop_statement ::=
         repeat ( expression ) statement

If *this* interpretation is used, then <statement> should be executed
5 times on the posedge of clk. The way the -1995 standard is written,
these are both equally valid interpretations of the example, yet they
produce very different results. The standard offers no guidance on how
to resolve this conflict, and the IEEE1364-2000 DRAFT does not improve
the situation.

Practice suggests that a repeat followed by an event control should be
interpreted as a loop head, and this is what Icarus Verilog does, as
well as all the other major Verilog tools, but the standard does not
say this.

* UNSIZED NUMERIC CONSTANTS ARE NOT LIMITED TO 32 BITS

The Verilog standard allows Verilog implementations to limit the size
of unsized constants to a bit width of at least 32. That means that a
constant 17179869183 (36'h3_ffff_ffff) may overflow some compilers. In
fact, it is common to limit these values to 32bits. However, a
compiler may just as easily choose another width limit, for example
64bits. That value is equally good.

However, it is not *required* that an implementation truncate at 32
bits, and in fact Icarus Verilog does not truncate at all. It will
make the unsized constant as big as it needs to be to hold the value
accurately. This is especially useful in situations like this::

	    reg [width-1:0] foo = 17179869183;

The programmer wants the constant to take on the width of the reg,
which in this example is parameterized. Since constant sizes cannot be
parameterized, the programmer ideally gives an unsized constant, which
the compiler then expands/contracts to match the l-value.

Also, by choosing to not ever truncate, Icarus Verilog can handle code
written for a 64bit compiler as easily as for a 32bit compiler. In
particular, any constants that the user does not expect to be
arbitrarily truncated by his compiler will also not be truncated by
Icarus Verilog, no matter what that other compiler chooses as a
truncation point.


* UNSIZED EXPRESSIONS AS PARAMETERS TO CONCATENATION {}

The Verilog standard clearly states in 4.1.14::

	"Unsized constant numbers shall not be allowed in
	concatenations. This is because the size of each
	operand in the concatenation is needed to calculate
	the complete size of the concatenation."

So for example the expression {1'b0, 16} is clearly illegal. It
also stands to reason that {1'b0, 15+1} is illegal, for exactly the
same justification. What is the size of the expression (15+1)?
Furthermore, it is reasonable to expect that (16) and (15+1) are
exactly the same so far as the compiler is concerned.

Unfortunately, Cadence seems to feel otherwise. In particular, it has
been reported that although {1'b0, 16} causes an error, {1'b0, 15+1}
is accepted. Further testing shows that any expression other than a
simple unsized constant is accepted there, even if all the operands of
all the operators that make up the expression are unsized integers.

This is a semantic problem. Icarus Verilog doesn't limit the size of
integer constants. This is valid as stated in 2.5.1 Note 3::

	"The number of bits that make up an unsized number
	(which is a simple decimal number or a number without
	the size specification) shall be *at*least* 32."
	[emphasis added]

Icarus Verilog will hold any integer constant, so the size will be as
large as it needs to be, whether that is 64bits, 128bits, or
more. With this in mind, what is the value of these expressions?

::

	{'h1_00_00_00_00}
	{'h1 << 32}
	{'h0_00_00_00_01 << 32}
	{'h5_00_00_00_00 + 1}

These examples show that the standard is justified in requiring that
the operands of concatenation have size. The dispute is what it takes
to cause an expression to have a size, and what that size is.
Verilog-XL claims that (16) does not have a size, but (15+1) does. The
size of the expression (15+1) is the size of the adder that is
created, but how wide is the adder when adding unsized constants?

One might note that the quote from section 4.1.14 says "Unsized
*constant*numbers* shall not be allowed." It does not say "Unsized
expressions...", so arguably accepting (15+1) or even (16+0) as an
operand to a concatenation is not a violation of the letter of the
law. However, the very next sentence of the quote expresses the
intent, and accepting (15+1) as having a more defined size than (16)
seems to be a violation of that intent.

Whatever a compiler decides the size is, the user has no way to
predict it, and the compiler should not have the right to treat (15+1)
any differently than (16). Therefore, Icarus Verilog takes the
position that such expressions are *unsized* and are not allowed as
operands to concatenations. Icarus Verilog will in general assume that
operations on unsized numbers produce unsized results. There are
exceptions when the operator itself does define a size, such as the
comparison operators or the reduction operators. Icarus Verilog will
generate appropriate error messages.


* MODULE INSTANCE WITH WRONG SIZE PORT LIST

A module declaration like this declares a module that takes three ports::

	module three (a, b, c);
	  input a, b, c;
	  reg x;
	endmodule

This is fine and obvious. It is also clear from the standard that
these are legal instantiations of this module::

	three u1 (x,y,z);
	three u2 ( ,y, );
	three u3 ( , , );
	three u4 (.b(y));

In some of the above examples, there are unconnected ports. In the
case of u4, the pass by name connects only port b, and leaves a and c
unconnected. u2 and u4 are the same thing, in fact, but using
positional or by-name syntax. The next example is a little less
obvious::

	three u4 ();

The trick here is that strictly speaking, the parser cannot tell
whether this is a list of no pass by name ports (that is, all
unconnected) or an empty positional list. If this were an empty
positional list, then the wrong number of ports is given, but if it is
an empty by-name list, it is an obviously valid instantiation. So it
is fine to accept this case as valid.

These are more doubtful::

	three u5(x,y);
	three u6(,);

These are definitely positional port lists, and they are definitely
the wrong length. In this case, the standard is not explicit about
what to do about positional port lists in module instantiations,
except that the first is connected to the first, second to second,
etc. It does not say that the list must be the right length, but every
example of unconnected ports used by-name syntax, and every example of
ordered list has the right size list.

Icarus Verilog takes the (very weak) hint that ordered lists should be
the right length, and will therefore flag instances u5 and u6 as
errors. The IEEE1364 standard should be more specific one way or the
other.

* UNKNOWN VALUES IN L-VALUE BIT SELECTS

Consider this example::

	reg [7:0] vec;
	wire [4:0] idx = <expr>;
	[...]
	vec[idx] = 1;

So long as the value of idx is a valid bit select address, the
behavior of this assignment is obvious. However, there is no explicit
word in the standard as to what happens if the value is out of
range. The standard clearly states the value of an expression when the
bit-select or part select is out of range (the value is x) but does
not address the behavior when the expression is an l-value.

Icarus Verilog will take the position that bit select expressions in
the l-value will select oblivion if it is out of range. That is, if
idx has a value that is not a valid bit select of vec, then the
assignment will have no effect.


* SCHEDULING VALUES IN LOGIC

The interaction between blocking assignments in procedural code and
logic gates in gate-level code and expressions is poorly defined in
Verilog. Consider this example::

   reg a;
   reg b;
   wire q = a & b;

   initial begin
      a = 1;
      b = 0;
      #1 b = 1;
      if (q !== 0) begin
	 $display("FAILED -- q changed too soon? %b", q);
	 $finish;
      end
   end

This is a confusing situation. It is clear from the Verilog standard
that an assignment to a variable using a blocking assign causes the
l-value to receive the value before the assignment completes. This
means that a subsequent read of the assigned variable *must* read back
what was blocking-assigned.

However, in the example above, the "wire q = a & b" expresses some
gate logic between a/b and q. The standard does not say whether a read
out of logic should read the value computed from previous assigns to
the input from the same thread. Specifically, when "a" and "b" are
assigned by blocking assignments, will a read of "q" get the computed
value or the existing value?

In fact, existing commercial tools do it both ways. Some tools print
the FAILED message in the above example, and some do not. Icarus
Verilog does not print the FAILED message in the above example,
because the gate value change is *scheduled* when inputs are assigned,
but not propagated until the thread gives up the processor.

Icarus Verilog chooses this behavior in order to filter out zero-width
pulses as early as possible. The implication of this is that a read of
the output of combinational logic will most likely *not* reflect the
changes in inputs until the thread that changed the inputs yields
execution.


* BIT AND PART SELECTS OF PARAMETERS

Bit and part selects are supposed to only be supported on vector nets
and variables (wires, regs, etc.) However, it is common for Verilog
compilers to also support bit and part select on parameters. Icarus
Verilog also chooses to support bit and part selects on parameter
names, but we need to define what that means.

A bit or a part select on a parameter expression returns an unsigned
value with a defined size. The parameter value is considered be a
constant vector of bits foo[X:0]. That is, zero based. The bit and
part selects operate from that assumption.

Verilog 2001 adds syntax to allow the user to explicitly declare the
parameter range (i.e. parameter [5:0] foo = 9;) so Icarus Verilog will
(or should) use the explicitly declared vector dimensions to interpret
bit and part selects.


* EDGES OF VECTORS

Consider this example::

   reg [ 5:0] clock;
   always @(posedge clock) [do stuff]

The IEEE1364 standard clearly states that the @(posedge clock) looks
only at the bit clock[0] (the least significant bit) to search for
edges. It has been pointed out by some that Verilog XL instead
implements it as `@(posedge |clock)`: it looks for a rise in the
reduction or of the vector. Cadence Design Systems technical support
has been rumored to claim that the IEEE1364 specification is wrong,
but NC-Verilog behaves according to the specification, and thus
different from XL.

Icarus Verilog, therefore, takes the position that the specification
is clear and correct, and it behaves as does NC-Verilog in this
matter.


* REAL VARIABLES IN $dumpoff DEAD-ZONES

The IEEE1364 standard clearly states that in VCD files, the $dumpoff
section checkpoints all the dumped variables as X values. For reg and
wire bits/vectors, this obviously means 'bx values. Icarus Verilog
does this, for example::

    $dumpoff
    x!
    x"
    $end

Real variables can also be included in VCD dumps, but it is not at
all obvious what is supposed to be dumped into the $dumpoff-$end
section of the VCD file. Verilog-XL dumps "r0 !" to set the real
variables to the dead-zone value of 0.0, whereas other tools, such as
ModelTech, ignore real variables in this section.

For example (from XL)::

    $dumpoff
    r0 !
    r0 "
    $end

Icarus Verilog dumps NaN values for real variables in the
$dumpoff-$end section of the VCD file. The NaN value is the IEEE754
equivalent of an unknown value, and so better reflects the unknown
(during the dead zone) status of the variable, like this::

    $dumpoff
    rNaN !
    rNaN "
    $end

It turns out that NaN is conventionally accepted by scanf functions,
and viewers that support real variables support NaN values. So while
the IEEE1364 doesn't require this behavior, and given the variety that
already seems to exist amongst VCD viewers in the wild, this behavior
seems to be acceptable according to the standard, is a better mirror
of 4-value behavior in the dead zone, and appears more user friendly
when viewed by reasonable viewers.
