
Loadable Target API (ivl_target)
================================

In addition to the standard VPI API, Icarus Verilog supports a non-standard
loadable target module API. This API helps C programmers write modules that
Icarus Verilog can use to generate code. These modules are used at compile
time to write the elaborated design to the simulation or netlist files. For
example, the vvp code generator is a loadable target module that writes vvp
code into the specified file.

Loadable target modules gain access to the 'elaborated' design. That means,
the source files have been checked for syntax and correctness, any synthesis
and general optimization steps have been performed, and what is left is a
design that reflects but is not exactly the same as the input Verilog source
code. This relieves the modules of the burden of supporting all the odd
corners and complexities of the Verilog language.

The Target Module API
---------------------

The API is defined in the header file "ivl_target.h" which is installed with
Icarus Verilog. The header defines the functions that the module writer can
use to get at the elaborated design during the course of writing the output
format.

The target module API function "target_design" is special in that the API does
not provide this function: The target module itself provides it. When the
compiler loads the target module, it invokes the "target_design" function with
a handle to the design. This is the point where the target module takes over
to process the design.

Compiling Target Modules
------------------------

Compiling loadable target modules is similar to compiling VPI modules, in that
the module must be compiled with the "-fPIC" flag to gcc, and linked with the
"-shared" flag. The module that you compile is then installed in a place where
the "iverilog" command can find it, and configuration files are adjusted to
account for the new module.

This code::

  # include  <ivl_target.h>

  int target_design(ivl_design_t des)
  {
       return 0;
  }

is an example module that we can write into the file "empty.c"; and let us
compile it into the module file "empty.tgt" like so::

  % gcc -o empty.tgt -fpic -shared empty.c

This makes the "empty.tgt" file an a dynamically loaded shared object.

Creating the Target Config File
-------------------------------

The target config file tells the Icarus Verilog core how to process your new
code generator. The ivl core expects two configuration files: the name.conf
and the name-s.config files. The "-s" version is what is used if the user
gives the "-S" (synthesis) flag on the command line.

The stub target, included in most distributions, demonstrates the config
files. The "stub.conf" file is::

  functor:cprop
  functor:nodangle
  -t:dll
  flag:DLL=stub.tgt

and the "stub-s.conf" file is::

  functor:synth2
  functor:synth
  functor:syn-rules
  functor:cprop
  functor:nodangle
  -t:dll
  flag:DLL=stub.tgt

Note that the "stub-s.conf" file contains more lines to invoke internal
synthesis functions, whereas the "stub.conf" invokes only the basic
optimization steps.

In general, only the last line (The "flag:DLL=<name>.tgt" record) varies for
each target. For your target, replace the <name> with the name of your target
and you have a configuration file ready to install. Note that this is the name
of your target module. This is in fact how the config file tells the compiler
the name of your module.

The rest of the config file is best taken as boiler plate and installed as is,
with one difference. If your target is a synthesis target (for example a mosis
code generator or a pld code generator) that expects synthesis to happen, then
it makes the most sense to create both your config file like the "stub-s.conf"
config file. This causes the compiler to do synthesis for your target whether
the user gives the "-S" flag or not.

Installing the Target Module
----------------------------

Finally, the "empty.conf", the "empty-s.conf" and the "empty.tgt" files need
to be installed. Where they go depends on your system, but in Linux they are
normally installed in "/usr/lib/ivl".


LPM Devices
-----------

All LPM devices support a small set of common LPM functions, as
described in the ivl_target header file. The ivl_lpm_t object has a
type enumerated by ivl_lpm_type_t, and that type is accessible via the
ivl_lpm_type function.

The following are type specific aspects of LPM devices.

* IVL_LPM_UFUNC

This LPM represents a user defined function. It is a way to connect
behavioral code into a structural network. The UFUNC device has a
vector output and a set of inputs. The ivl_lpm_define function returns
the definition as an ivl_scope_t object.

The output vector is accessible through the ivl_lpm_q, and the output
has the width defined by ivl_lpm_width. This similar to most every
other LPM device with outputs.

There are ivl_lpm_size() input ports, each with the width
ivl_lpm_data2_width(). The actual nexus is indexed by ivl_lpm_data2().
