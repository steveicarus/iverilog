
Simulation Using Icarus Verilog
===============================

Simulation is the process of creating models that mimic the behavior of the
device you are designing (simulation models) and creating models to exercise
the device (test benches). The simulation model need not reflect any
understanding of the underlying technology, and the simulator need not know
that the design is intended for any specific technology.

The Verilog simulator, in fact, is usually a different program than the
synthesizer. It may even come from a different vendor. The simulator need not
know of or generate netlists for the target technology, so it is possible to
write one simulator that can be used to model designs intended for a wide
variety of technologies. A synthesizer, on the other hand, does need to know a
great deal about the target technology in order to generate efficient
netlists. Synthesizers are often technology specific and come from vendors
with specialized knowledge, whereas simulators are more general purpose.

Simulation models and test benches, therefore, can use the full range of
Verilog features to model the intended design as clearly as possible. This is
the time to test the algorithms of the design using language that is
relatively easy for humans to read. The simulator, along with the test bench,
can test that the clearly written model really does behave as intended, and
that the intended behavior really does meet expectations.

The test benches model the world outside the design, so they are rarely
destined for real hardware. They are written in Verilog simply as a matter of
convenience, and sometimes they are not written in Verilog at all. The test
benches are not throw-away code either, as they are used to retest the device
under test as it is transformed from a simulation model to a synthesizeable
description.

Compilation and Elaboration
---------------------------

Simulation of a design amounts to compiling and executing a program. The
Verilog source that represents the simulation model and the test bench is
compiled into an executable form and executed by a simulation
engine. Internally, Icarus Verilog divides the compilation of program source
to an executable form into several steps, and basic understanding of these
steps helps understand the nature of failures and errors. The first step is
macro preprocessing, then compilation, elaboration, optional optimizations and
finally code generation. The boundary between these steps is often blurred,
but this progression remains a useful model of the compilation process.

The macro preprocessing step performs textual substitutions of macros defined
with "\`define" statements, textual inclusion with "\`include" statements, and
conditional compilation by "\`ifdef" and "\`ifndef" statements. The
macropreprocessor for Icarus Verilog is internally a separate program that can
be accessed independently by using the "-E" flag to the "iverilog" command,
like so::

  % iverilog -E -o out.v example.v

This command causes the input Verilog file "example.v" to be preprocessed, and
the output, a Verilog file without preprocessor statements, written into
"out.v". The "\`include" and "\`ifdef" directives in the input file are interpreted,
and defined macros substituted, so that the output, a single file, is the same
Verilog but with the preprocessor directives gone. All the explicitly
specified source files are also combined by the preprocessor, so that the
preprocessed result is a single Verilog stream.

Normally, however, the "-E" flag is not used and the preprocessed Verilog is
instead sent directly to the next step, the compiler. The compiler core takes
as input preprocessed Verilog and generates an internal parsed form. The
parsed form is an internal representation of the Verilog source, in a format
convenient for further processing, and is not accessible to the user.

The next step, elaboration, takes the parsed form, chooses the root modules,
and instantiates (makes *instances* of) those roots. The root instances may
contain instances of other modules, which may in turn contain instances of yet
other modules. The elaboration process creates a hierarchy of module instances
that ends with primitive gates and statements.

Note that there is a difference between a module and a module instance. A
module is a type. It is a description of the contents of module instances that
have its type. When a module is instantiated within another module, the module
name identifies the type of the instance, and the instance name identifies the
specific instance of the module. There can be many instances of any given
module.

Root modules are a special case, in that the programmer does not give them
instance names. Instead, the instance names of root modules are the same as
the name of the module. This is valid because, due to the nature of the
Verilog syntax, a module can be a root module only once, so the module name
itself is a safe instance name.

Elaboration creates a hierarchy of scopes. Each module instance creates a new
scope within its parent module, with each root module starting a
hierarchy. Every module instance in the elaborated program has a unique scope
path, a hierarchical name, that starts with its root scope and ends with its
own instance name. Every named object, including variables, parameters, nets
and gates, also has a hierarchical name that starts with a root scope and ends
with its own base name. The compiler uses hierarchical names in error messages
generated during or after elaboration, so that erroneous items can be
completely identified. These hierarchical names are also used by waveform
viewers that display waveform output from simulations.

The elaboration process creates from the parsed form the scope hierarchy
including the primitive objects within each scope. The elaborated design then
is optimized to reduce it to a more optimal, but equivalent design. The
optimization step takes the fully elaborated design and transforms it to an
equivalent design that is smaller or more efficient. These optimizations are,
for example, forms of constant propagation and dead code elimination. Useless
logic is eliminated, and constant expressions are pre-calculated. The
resulting design behaves as if the optimizations were not performed, but is
smaller and more efficient. The elimination (and spontaneous creation) of
gates and statements only affects the programmer when writing VPI modules,
which through the API have limited access to the structures of the design.

Finally, the optimized design, which is still in an internal form not
accessible to users, is passed to a code generator that writes the design into
an executable form. For simulation, the code generator is selected to generate
the vvp format--a text format that can be executed by the simulation
engine. Other code generators may be selected by the Icarus Verilog user, even
third party code generators, but the vvp code generator is the default for
simulation purposes.

Making and Using Libraries
--------------------------

Although simple programs may be written into a single source file, this gets
inconvenient as the designs get larger. Also, writing the entire program into
a single file makes it difficult for different programs to share common
code. It therefore makes sense to divide large programs into several source
files, and to put generally useful source code files somewhere accessible to
multiple designs.

Once the program is divided into many files, the compiler needs to be told how
to find the files of the program. The simplest way to do that is to list the
source files on the command line or in a command file. This is for example the
best way to divide up and integrate test bench code with the simulation model
of the device under test.

The Macro Preprocessor
^^^^^^^^^^^^^^^^^^^^^^

Another technique is to use the macro preprocessor to include library files
into a main file. The `include` directive takes the name of a source file to
include. The preprocessor inserts the entire contents of the included file in
place of the `include` directive. The preprocessor normally looks in the
current working directory (the current working directory of the running
compiler, and not the directory where the source file is located) for the
included file, but the "-I" switch to "iverilog" can add directories to the
search locations list. ::

  % iverilog -I/directory/to/search example.v

It is common to create include directories shared by a set of programs. The
preprocessor `include` directive can be used by the individual programs to
include the source files that it needs.

The preprocessor method of placing source code into libraries is general
(arbitrary source code can be placed in the included files) but is static, in
the sense that the programmer must explicitly include the desired library
files. The automatic module library is a bit more constrained, but is
automatic.

Automatic Module Libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^

A common use for libraries is to store module definitions that may be of use
to a variety of programs. If modules are divided into a single module per
file, and the files are named appropriately, and the compiler is told where to
look, then the compiler can automatically locate library files when it finds
that a module definition is missing.

For this to work properly, the library files must be Verilog source, they
should contain a single module definition, and the files must be named after
the module they contain. For example, if the module "AND2" is a module in the
library, then it belongs in a file called "AND2.v" and that file contains only
the "AND2" module. A library, then, is a directory that contains properly
named and formatted source files. ::

  % iverilog -y/library/to/search example.v

The "-y" flag to "iverilog" tells the compiler to look in the specified
directory for library modules whenever the program instantiates a module that
is not otherwise defined. The programmer may include several "-y" flags on the
command line (or in a command file) and the compiler will search each
directory in order until an appropriate library file is found to resolve the
module.

Once a module is defined, either in the program or by reading a library
module, the loaded definition is used from then on within the program. If the
module is defined within a program file or within an included file, then the
included definition is used instead of any library definition. If a module is
defined in multiple libraries, then the first definition that the compiler
finds is used, and later definitions are never read.

Icarus Verilog accesses automatic libraries during elaboration, after it has
already preprocessed and parsed the non-library source files. Modules in
libraries are not candidates for root modules, and are not even parsed unless
they are instantiated in other source files. However, a library module may
reference other library modules, and reading in a library module causes it to
be parsed and elaborated, and further library references resolved, just like a
non-library module. The library lookup and resolution process iterates until
all referenced modules are resolved, or known to be missing from the
libraries.

The automatic module library technique is useful for including vendor or
technology libraries into a program. Many EDA vendors offer module libraries
that are formatted appropriately; and with this technique, Icarus Verilog can
use them for simulation.

Advanced Command Files
----------------------

Command files were mentioned in the "Getting Started" chapter, but only
briefly. In practice, Verilog programs quickly grow far beyond the usefulness
of simple command line options, and even the macro preprocessor lacks the
flexibility to combine source and library modules according to the advancing
development process.

The main contents of a command file is a list of Verilog source files. You can
name in a command file all the source files that make up your design. This is
a convenient way to collect together all the files that make up your
design. Compiling the design can then be reduced to a simple command line like
the following::

  % iverilog -c example.cf

The command file describes a configuration. That is, it lists the specific
files that make up your design. It is reasonable, during the course of
development, to have a set of different but similar variations of your
design. These variations may have different source files but also many common
source files. A command file can be written for each variation, and each
command file lists the source file names used by each variation.

A configuration may also specify the use of libraries. For example, different
configurations may be implementations for different technologies so may use
different parts libraries. To make this work, command files may include "-y"
statements. These work in command files exactly how they work on "iverilog"
command line. Each "-y" flag is followed by a directory name, and the
directories are searched for library modules in the order that they are listed
in the command file.

The include search path can also be specified in configuration files with
"+incdir+" tokens. These tokens start with the "+incdir+" string, then
continue with directory paths, separated from each other with "+" characters
(not spaces) for the length of the line.

Other information can be included in the command file. See the section Command
File Format for complete details on what can go in a command file.

Input Data at Runtime
---------------------

Often, it is useful to compile a program into an executable simulation, then
run the simulation with various inputs. This requires some means to pass data
and arguments to the compiled program each time it is executed. For example,
if the design models a micro-controller, one would like to run the compiled
simulation against a variety of different ROM images.

There are a variety of ways for a Verilog program to get data from the outside
world into the program at run time. Arguments can be entered on the command
line, and larger amounts of data can be read from files. The simplest method
is to take arguments from the command line.

Consider this running example of a square root calculator

.. code-block:: verilog

  module sqrt32(clk, rdy, reset, x, .y(acc));
    input  clk;
    output rdy;
    input  reset;

    input [31:0] x;
    output [15:0] acc;

    // acc holds the accumulated result, and acc2 is
    //  the accumulated square of the accumulated result.
    reg [15:0] acc;
    reg [31:0] acc2;

    // Keep track of which bit I'm working on.
    reg [4:0]  bitl;
    wire [15:0] bit = 1 << bitl;
    wire [31:0] bit2 = 1 << (bitl << 1);

    // The output is ready when the bitl counter underflows.
    wire rdy = bitl[4];

    // guess holds the potential next values for acc,
    // and guess2 holds the square of that guess.
    wire [15:0] guess  = acc | bit;
    wire [31:0] guess2 = acc2 + bit2 + ((acc << bitl) << 1);

    task clear;
       begin
          acc = 0;
          acc2 = 0;
          bitl = 15;
       end
    endtask

    initial clear;

    always @(reset or posedge clk)
       if (reset)
        clear;
       else begin
          if (guess2 <= x) begin
             acc  <= guess;
             acc2 <= guess2;
          end
          bitl <= bitl - 1;
       end

  endmodule

One could write the test bench as a program that passes a representative set
of input values into the device and checks the output result. However, we can
also write a program that takes on the command line an integer value to be
used as input to the device. We can write and compile this program, then pass
different input values on the run time command line without recompiling the
simulation.

This example demonstrates the use of the "$value$plusargs" to access command
line arguments of a simulation

.. code-block:: verilog

  module main;

    reg clk, reset;
    reg [31:0] x;
    wire [15:0] y;
    wire        rdy;

    sqrt32 dut (clk, rdy, reset, x, y);

    always #10 clk = ~clk;

    initial begin
       clk = 0;
       reset = 1;

       if (! $value$plusargs("x=%d", x)) begin
          $display("ERROR: please specify +x=<value> to start.");
          $finish;
       end

       #35 reset = 0;

       wait (rdy) $display("y=%d", y);
       $finish;
    end // initial begin

  endmodule // main

The "$value$plusargs" system function takes a string pattern that describes
the format of the command line argument, and a reference to a variable that
receives the value. The "sqrt_plusargs" program can be compiled and executed
like this::

  % iverilog -osqrt_plusargs.vvp sqrt_plusargs.v sqrt.v
  % vvp sqrt_plusargs.vvp +x=81
  y=    9

Notice that the "x=%d" string of the "$value$plusargs" function describes the
format of the argument. The "%d" matches a decimal value, which in the sample
run is "81". This gets assigned to "x" by the "$value$plusargs" function,
which returns TRUE, and the simulation continues from there.

If two arguments have to be passed to the testbench then the main module would
be modified as follows

.. code-block:: verilog

  module main;

    reg clk, reset;
    reg  [31:0] x;
    reg  [31:0] z;
    wire [15:0] y1,y2;
    wire        rdy1,rdy2;

    sqrt32 dut1 (clk, rdy1, reset, x, y1);
    sqrt32 dut2 (clk, rdy2, reset, z, y2);

    always #10 clk = ~clk;

    initial begin
       clk = 0;
       reset = 1;

       if (! $value$plusargs("x=%d", x)) begin
          $display("ERROR: please specify +x=<value> to start.");
          $finish;
       end

       if (! $value$plusargs("z=%d", z)) begin
          $display("ERROR: please specify +z=<value> to start.");
          $finish;
       end


       #35 reset = 0;

       wait (rdy1) $display("y1=%d", y1);
       wait (rdy2) $display("y2=%d", y2);
       $finish;
    end // initial begin

  endmodule // main

and the "sqrt_plusargs" program would be compiled and executed as follows::

  % iverilog -osqrt_plusargs.vvp sqrt_plusargs.v sqrt.v
  % vvp sqrt_plusargs.vvp +x=81 +z=64
  y1=    9
  y2=    8

In general, the "vvp" command that executes the compiled simulation takes a
few predefined argument flags, then the file name of the simulation. All the
arguments after the simulation file name are extended arguments to "vvp" and
are passed to the executed design. Extended arguments that start with a "+"
character are accessible through the "$test$plusargs" and "$value$plusargs"
system functions. Extended arguments that do not start with a "+" character
are only accessible to system tasks and functions written in C using the VPI.

In the previous example, the program pulls the argument from the command line,
assigns it to the variable "x", and runs the sqrt device under test with that
value. This program can take the integer square root of any single value. Of
course, if you wish to test with a large number of input values, executing the
program many times may become tedious.

Another technique would be to put a set of input values into a data file, and
write the test bench to read the file. We can then edit the file to add new
input values, then rerun the simulation without compiling it again. The
advantage of this technique is that we can accumulate a large set of test
input values, and run the lot as a batch.

This example

.. code-block:: verilog

  module main;

    reg clk, reset;
    reg [31:0] data[4:0];
    reg [31:0] x;
    wire [15:0] y;
    wire        rdy;

    sqrt32 dut (clk, rdy, reset, x, y);

    always #10 clk = ~clk;

    integer i;
    initial begin
       /* Load the data set from the hex file. */
       $readmemh("sqrt.hex", data);
       for (i = 0 ;  i <= 4 ;  i = i + 1) begin
         clk = 0;
         reset = 1;

         x = data[i];

         #35 reset = 0;

         wait (rdy) $display("y=%d", y);
       end
       $finish;
    end // initial begin

  endmodule // main

demonstrates the use of "$readmemh" to read data samples from a file into a
Verilog array. Start by putting into the file "sqrt.hex" the numbers::

  51
  19
  1a
  18
  1

Then run the simulation with the command sequence::

  % iverilog -osqrt_readmem.vvp sqrt_readmem.vl sqrt.vl
  % vvp sqrt_readmem.vvp
  y=    9
  y=    5
  y=    5
  y=    4
  y=    1

It is easy enough to change this program to work with larger data sets, or to
change the "data.hex" file to contain different data. This technique is also
common for simulating algorithms that take in larger data sets. One can extend
this idea slightly by using a "$value$plusargs" statement to select the file
to read.
