
Developer Guide
===============

The developer guide is intended to give you a gross structure of the
Icarus Verilog compiler source. This will help orient you to the
source code itself, so that you can find the global parts where you
can look for even better detail.

The documentation for getting, building and installing Icarus Verilog
is kept and maintained at :doc:`Getting Started as a Contributer <../getting_started>`

See the Installation Guide for getting the current source from the git
repository (and how to use the git repository) and see the Developer Guide
for instructions on participating in the Icarus Verilog development process.
That information will not be repeated here.

Scroll down to a listing with further readings.

Compiler Components
-------------------

- The compiler driver (driver/)

This is the binary that is installed as "iverilog". This program takes
the command line arguments and assembles invocations of all the other
subcommands to perform the steps of compilation.

- The preprocessor (ivlpp/)

This implements the Verilog pre-processor. In Icarus Verilog, the
compiler directives \`define, \`include, \`ifdef and etc. are implemented
in an external program. The ivlpp/ directory contains the source for
this program.

- The core compiler (root directory)

The "ivl" program is the core that does all the Verilog compiler
processing that is not handled elsewhere. This is the main core of the
Icarus Verilog compiler, not the runtime. See below for more details
on the core itself.

- The loadable code generators (tgt-\*/)

This core compiler, after it is finished with parsing and semantic
analysis, uses loadable code generators to emit code for supported
targets. The tgt-\*/ directories contains the source for the target
code generators that are bundled with Icarus Verilog. The tgt-vvp/
directory in particular contains the code generator for the vvp
runtime.


Runtime Components
------------------

- The vvp runtime (vvp/)

This program implements the runtime environment for Icarus
Verilog. It implements the "vvp" command described in the user
documentation. See the vvp/ subdirectory for further developer
documentation.

- The system tasks implementations (vpi/)

The standard Verilog system tasks are implemented using VPI (PLI-2)
and the source is in this subdirectory.

- The PLI-1 compatibility library (libveriuser/)

The Icarus Verilog support for the deprecated PLI-1 is in this
subdirectory. The vvp runtime does not directly support the
PLI-1. Instead, the libveriuser library emulates it using the builtin
PLI-2 support.

- The Cadence PLI module compatibility module (cadpli/)

It is possible in some specialized situations to load and execute
PLI-1 code written for Verilog-XL. This directory contains the source
for the module that provides the Cadence PLI interface.


The Core Compiler
-----------------

The "ivl" binary is the core compiler that does the heavy lifting of
compiling the Verilog source (including libraries) and generating the
output. This is the most complex component of the Icarus Verilog
compilation system.

The process in the abstract starts with the Verilog lexical analysis
and parsing to generate an internal "pform". The pform is then
translated by elaboration into the "netlist" form. The netlist is
processed by some functors (which include some optimizations and
optional synthesis) then is translated into the ivl_target internal
form. And finally, the ivl_target form is passed via the ivl_target.h
API to the code generators.

- Lexical Analysis

Lexical analysis and parsing use the tools "flex", "gperf", and
"bison". The "flex" input file "lexor.lex" recognizes the tokens in
the input stream. This is called "lexical analysis". The lexical
analyzer also does some processing of compiler directives that are not
otherwise taken care of by the external preprocessor. The lexical
analyzer uses a table of keywords that is generated using the "gperf"
program and the input file "lexor_keywords.gperf". This table allows
the lexical analyzer to efficiently check input words with the rather
large set of potential keywords.

- Parsing

The parser input file "parse.y" is passed to the "bison" program to
generate the parser. The parser uses the functions in parse*.h,
parse*.cc, pform.h, and pform*.cc to generate the pform from the
stream of input tokens. The pform is what compiler writers call a
"decorated parse tree".

The pform itself is described by the classes in the header files
"PScope.h", "Module.h", "PGenerate.h", "Statement.h", and
"PExpr.h". The implementations of the classes in those header files
are in the similarly named C++ files.

- Elaboration

Elaboration transforms the pform to the netlist form. Elaboration is
conceptually divided into several major steps: Scope elaboration,
parameter overrides and defparam propagation, signal elaboration, and
statement and expression elaboration.

The elaboration of scopes and parameter overrides and defparam
propagation are conceptually separate, but are in practice
intermingled. The elaboration of scopes scans the pform to find and
instantiate all the scopes of the design. New scopes are created by
instantiation of modules (starting with the root instances) by user
defined tasks and functions, named blocks, and generate schemes. The
elaborate_scope methods implement scope elaboration, and the
elab_scope.cc source file has the implementations of those
methods.

The elaborate.cc source file contains the initial calls to the
elaborate_scope for the root scopes to get the process started. In
particular, see the "elaborate" function near the bottom of the
elaborate.cc source file. The calls to Design::make_root_scope create
the initial root scopes, and the creation and enqueue of the
elaborate_root_scope_t work items primes the scope elaboration work
list.

Intermingled in the work list are defparms work items that call the
Design::run_defparams and Design::evaluate_parameters methods that
override and evaluate parameters. The override and evaluation of
parameters must be intermingled with the elaboration of scopes because
the exact values of parameters may impact the scopes created (imagine
generate schemes and instance arrays) and the created scopes in turn
create new parameters that need override and evaluation.

Further Reading
---------------

For further information on the individual parts of Icarus Verilog, see this listing:

.. toctree::
   :maxdepth: 2

   ivl/index
   vvp/index
   tgt-vvp/tgt-vvp
   vpi/index
   cadpli/cadpli
   misc/index
