
Using VPI
=========

Icarus Verilog implements a portion of the PLI 2.0 API to Verilog. This allows
programmers to write C code that interfaces with Verilog simulations to
perform tasks otherwise impractical with straight Verilog. Many Verilog
designers, especially those who only use Verilog as a synthesis tool, can
safely ignore the entire matter of the PLI (and this chapter) but the designer
who wishes to interface a simulation with the outside world cannot escape VPI.

The rest of this article assumes some knowledge of C programming, Verilog PLI,
and of the compiler on your system. In most cases, Icarus Verilog assumes the
GNU Compilation System is the compiler you are using, so tips and instructions
that follow reflect that. If you are not a C programmer, or are not planning
any VPI modules, you can skip this entire article. There are references at the
bottom for information about more general topics.

How It Works
------------

The VPI modules are compiled loadable object code that the runtime loads at
the user's request. The user commands vvp to locate and load modules with the
"-m" switch. For example, to load the "sample.vpi" module::

  % vvp -msample foo.vvp

The vvp run-time loads the modules first, before executing any of the
simulation, or even before compiling the vvp code. Part of the loading
includes invoking initialization routines. These routines register with the
run-time all the system tasks and functions that the module implements. Once
this is done, the run time loader can match names of the called system tasks
of the design with the implementations in the VPI modules.

(There is a special module, the system.vpi module, that is always loaded to
provide the core system tasks.)

The simulator run time (The "vvp" program) gets a handle on a freshly loaded
module by looking for the symbol "vlog_startup_routines" in the loaded
module. This table, provided by the module author and compiled into the
module, is a null terminated table of function pointers. The simulator calls
each of the functions in the table in order. The following simple C definition
defines a sample table::

  void (*vlog_startup_routines[])() = {
     hello_register,
     0
  };

Note that the "vlog_startup_routines" table is an array of function pointers,
with the last pointer a 0 to mark the end. The programmer can organize the
module to include many startup functions in this table, if desired.

The job of the startup functions that are collected in the startup table is to
declare the system tasks and functions that the module provides. A module may
implement as many tasks/functions as desired, so a module can legitimately be
called a library of system tasks and functions.

Compiling VPI Modules
---------------------

To compile and link a VPI module for use with Icarus Verilog, you must compile
all the source files of a module as if you were compiling for a DLL or shared
object. With gcc under Linux, this means compiling with the "-fpic" flag. The
module is then linked together with the vpi library like so::

  % gcc -c -fpic hello.c
  % gcc -shared -o hello.vpi hello.o -lvpi

This assumes that the "vpi_user.h header file and the libvpi.a library file
are installed on your system so that gcc may find them. This is normally the
case under Linux and UNIX systems. An easier, the preferred method that works
on all supported systems is to use the single command::

  % iverilog-vpi hello.c

The "iverilog-vpi" command takes as command arguments the source files for
your VPI module, compiles them with proper compiler flags, and links them into
a vpi module with any system specific libraries and linker flags that are
required. This simple command makes the "hello.vpi" module with minimum fuss.

A Worked Example
----------------

Let us try a complete, working example. Place the C code that follows into the
file hello.c::

  # include  <vpi_user.h>

  static int hello_compiletf(char*user_data)
  {
        return 0;
  }

  static int hello_calltf(char*user_data)
  {
        vpi_printf("Hello, World!\n");
        return 0;
  }

  void hello_register()
  {
        s_vpi_systf_data tf_data;

        tf_data.type      = vpiSysTask;
        tf_data.tfname    = "$hello";
        tf_data.calltf    = hello_calltf;
        tf_data.compiletf = hello_compiletf;
        tf_data.sizetf    = 0;
        tf_data.user_data = 0;
        vpi_register_systf(&tf_data);
  }

  void (*vlog_startup_routines[])() = {
      hello_register,
      0
  };

and place the Verilog code that follows into hello.v::

  module main;
    initial $hello;
  endmodule

Next, compile and execute the code with these steps::

  % iverilog-vpi hello.c
  % iverilog -ohello.vvp hello.v
  % vvp -M. -mhello hello.vvp
  Hello, World!

The compile and link in this example are conveniently combined into the
"iverilog-vpi" command. The "iverilog" command then compiles the "hello.v"
Verilog source file to the "hello.vvp" program. Next, the "vvp" command
demonstrates the use of the "-M" and "-m" flags to specify a vpi module search
directory and vpi module name. Specifically, they tell the "vvp" command where
to find the module we just compiled.

The "vvp" command, when executed as above, loads the "hello.vpi" module that
it finds in the current working directory. When the module is loaded, the
vlog_startup_routines table is scanned, and the "hello_register" function is
executed. The "hello_register" function in turn tells "vvp" about the system
tasks that are included in this module.

After the modules are all loaded, the "hello.vvp" design file is loaded and
its call to the "$hello" system task is matched up to the version declared by
the module. While "vvp" compiles the "hello.vvp" source, any calls to "$hello"
are referred to the "compiletf" function. This function is called at compile
time and can be used to check parameters to system tasks or function. It can
be left empty like this, or left out completely. The "compiletf" function can
help performance by collecting parameter checks in compile time, so they do
not need to be done each time the system task is run, thus potentially saving
execution time overall.

When the run-time executes the call to the hello system task, the
"hello_calltf" function is invoked in the loaded module, and thus the output
is generated. The "calltf" function is called at run time when the Verilog
code actually executes the system task. This is where the active code of the
task belongs.

System Function Return Types
----------------------------

Icarus Verilog supports system functions as well as system tasks, but there is
a complication. Notice how the module that you compile is only loaded by the
"vvp" program. This is mostly not an issue, but elaboration of expressions
needs to keep track of types, so the main compiler needs to know the return
type of functions.

Starting with Icarus Verilog v11, the solution is quite simple. The names and
locations of the user's VPI modules can be passed to the compiler via the
"iverilog" -m and -L flags and the IVERILOG_VPI_MODULE_PATH environment
variable. The compiler will load and analyse the specified modules to
automatically determine any function return types. The compiler will also
automatically pass the names and locations of the specified modules to the
"vvp" program, so that they don't need to be specified again on the "vvp"
command line.

For Icarus Verilog versions prior to v11, the solution requires that the
developer of a module include the table in a form that the compiler can
read. The System Function Table file carries this information. A simple
example looks like this::

  # Example sft declarations of some common functions
  $random      vpiSysFuncInt
  $bitstoreal  vpiSysFuncReal
  $realtobits  vpiSysFuncSized 64 unsigned

This demonstrates the format of the file and support types. Each line contains
a comment (starts with "#") or a type declaration for a single function. The
declaration starts with the name of the system function (including the leading
"$") and ends with the type. The supported types are:

* vpiSysFuncInt
* vpiSysFuncReal
* vpiSysFuncSized <wid> <signed|unsigned>

Any functions that do not have an explicit type declaration in an SFT file are
implicitly taken to be "vpiSysFuncSized 32 unsigned".

The module author provides, along with the ".vpi" file that is the module, a
".sft" that declares all the function return types. For example, if the file
is named "example.sft", pass it to the "iverilog" command line or in the
command file exactly as if it were an ordinary source file.

Cadence PLI Modules
-------------------

With the cadpli module, Icarus Verilog is able to load PLI1 applications that
were compiled and linked to be dynamic loaded by Verilog-XL or
NC-Verilog. This allows Icarus Verilog users to run third-party modules that
were compiled to interface with XL or NC. Obviously, this only works on the
operating system that the PLI application was compiled to run on. For example,
a Linux module can only be loaded and run under Linux. In addition, a 64-bit
version of vvp can only load 64-bit PLI1 applications, etc.

Icarus Verilog uses an interface module, the "cadpli" module, to connect the
worlds. This module is installed with Icarus Verilog, and is invoked by the
usual -m flag to iverilog or vvp. This module in turn scans the extended
arguments, looking for -cadpli= arguments. The latter specify the share object
and bootstrap function for running the module. For example, to run the module
product.so, that has the bootstrap function "my_boot"::

  % vvp -mcadpli a.out -cadpli=./product.so:my_boot

The "-mcadpli" argument causes vvp to load the cadpli.vpl library module. This
activates the -cadpli= argument interpreter. The -cadpli=<module>:<boot_func>
argument, then, causes vvp, through the cadpli module, to load the loadable
PLI application, invoke the my_boot function to get a veriusertfs table, and
scan that table to register the system tasks and functions exported by that
object. The format of the -cadpli= extended argument is essentially the same
as the +loadpli1= argument to Verilog-XL.

The integration from this point is seamless. The PLI application hardly knows
that it is being invoked by Icarus Verilog instead of Verilog-XL, so operates
as it would otherwise.

Other References
----------------

Since the above only explains how to get PLI/VPI working with Icarus Verilog,
here are some references to material to help with the common aspects of
PLI/VPI.

* Principles of Verilog PLI by Swapnajit Mittra. ISBN 0-7923-8477-6
* The Verilog PLI Handbook by Stuart Sutherland. ISBN 0-7923-8489-X
