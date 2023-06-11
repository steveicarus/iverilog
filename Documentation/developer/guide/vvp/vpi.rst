
VPI Within VVP
==============

System tasks and functions in Verilog are implemented in Icarus
Verilog by C routines written with VPI. This implies that the vvp
engine must provide at least a subset of the Verilog VPI
interface. The minimalist concepts of vvp, however, make the method
less than obvious.

Within a Verilog design, there is a more or less fixed web of
vpiHandles that is the design database as is available to VPI
functions. The Verilog standard defines quite a lot of types, but the
vvp only implements the ones it needs. The VPI web is added into the
design using special pseudo-ops that create the needed objects.


Loading VPI Modules
-------------------

The vvp runtime loads VPI modules at runtime before the parser reads
in the source files. This gives the modules a chance to register tasks
and functions before the source is compiled. This allows the compiler
to resolve references to system tasks and system functions to a
vpiHandle at compile time. References to missing tasks/function can
thus be caught before the simulation is run.

     NOTE: This also, miraculously, allows for some minimal support of
     the compiletf call. From the perspective of VPI code, compilation
     of the VVP source is not unlike compilation of the original
     Verilog.

The handle that the vvp threads have to the VPI are the vpiHandles of
the system tasks and functions. The %vpi_call instruction, once compiled,
carries the vpiHandle of the system task.


System Task Calls
-----------------

A system task call invokes a VPI routine, and makes available to that
routine the arguments to the system task. The called routine gets
access to the system task call by calling back the VPI requesting the
handle. It uses the handle, in turn, to get hold of the operands for
the task.

All that vvp needs to know about a system task call is the handle of
the system task definitions (created by the vpi_register_systf
function) and the arguments of the actual call. The arguments are
tricky because the list has no bound, even though each particular call
in the Verilog source has a specific set of parameters.

Since each call takes a fixed number of parameters, the input source
can include in the statement the list of arguments. The argument list
will not fit in a single generated instruction, but a vpiHandle that
refers to a vpiSysTfCall does. Therefore, the compiler can take the
long argument list and form a vpiSysTaskCall object. The generated
instruction then only needs to be a %vpi_call with the single parameter
that is the vpiHandle for the call.


System Function Calls
---------------------

System function calls are similar to system tasks. The only
differences are that all the arguments are input only, and there is a
single magic output that is the return value. The same %vpi_call can
even be used to call a function.

System functions, like system tasks, can only be called from thread
code. However, they can appear in expressions, even when that
expression is entirely structural. The desired effect is achieved by
writing a wrapper thread that calls the function when inputs change,
and that writes the output into the containing expression.


System Task/Function Arguments
------------------------------

The arguments to each system task or call are not stored in the
instruction op-code, but in the vpiSysTfCall object that the compiler
creates and that the %vpi_call instruction ultimately refers to. All
the arguments must be some sort of object that can be represented by a
vpiHandle at compile time.

Arguments are handled at compile time by the parser, which uses the
argument_list rule to build a list of vpiHandle objects. Each argument
in the argument_list invokes whatever function is appropriate for the
token in order to make a vpiHandle object for the argument_list. When
all this is done, an array of vpiHandles is passed to code to create a
vpiSysTfCall object that has all that is needed to make the call.


Scopes
------

VPI can access scopes as objects of type vpiScope. Scopes have names
and can also contain other sub-scopes, all of which the VPI function
can access by the vpiInternalScope reference. Therefore, the run-time
needs to form a tree of scopes into which other scoped VPI objects are
placed.

A scope is created with a .scope directive, like so::

	<label> .scope "name" [, <parent>];
		.timescale <units>;

The scope takes a string name as the first parameter. If there is an
additional parameter, it is a label of the directive for the parent
scope. Scopes that have no parent are root scopes. It is an error to
declare a scope with the same name more than once in a parent scope.

The name string given when creating the scope is the basename for the
scope. The vvp automatically constructs full names from the scope
hierarchy, and runtime VPI code can access that full name with the
vpiFullName reference.

The .timescale directive changes the scope units from the simulation
precision to the specified precision. The .timescale directive affects
the current scope.

Objects that place themselves in a scope place themselves in the
current scope. The current scope is the one that was last mentioned by
a .scope directive. If the wrong scope is current, the label on a
scope directive can be used to resume a scope. The syntax works like
this::

		.scope <symbol>;

In this case, the <symbol> is the label of a previous scope directive,
and is used to identify the scope to be resumed. A scope resume
directive cannot have a label.


Variables
---------

Reg vectors (scalars are vectors of length 1) are created by .var
statements in the source. The .var statement includes the declared
name of the variable, and the indices of the MSB and LSB of the
vector. The vpiHandle is then created with this information, and the
vpi_ipoint_t pointer to the LSB functor of the variable. VPI programs
access the vector through the vpiHandle and related functions. The VPI
code gets access to the declared indices.

The VPI interface to variable (vpiReg objects) uses the MSB and LSB
values that the user defined to describe the dimensions of the
object.

::

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
     *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
     */
