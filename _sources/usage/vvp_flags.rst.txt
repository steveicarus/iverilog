VVP Command Line Flags
======================

The vvp command is the simulation run-time engine. The command line for vvp
execution is first the options and flags, then the vvp input file, and finally
extended arguments. Typical usage looks like this::

  % vvp <flags> foo.vvp <extended arguments>

Options/Flags
-------------

These options/flags go before the path to the vvp-executable program. They
effect behavior of the vvp runtime engine, including preparation for
simulation.

* -l<logfile>

  This flag specifies a logfile where all MCI <stdlog> output goes. Specify
  logfile as '-' to send log output to <stderr>. $display and friends send
  their output both to <stdout> and <stdlog>.

* -M<path>

  Add the directory path to the (VPI) module search path. Multiple "-M" flags
  are allowed, and the directories are added in the order that they are given
  on the command line.

  The string "-M-" is special, in that it doesn't add a directory to the
  path. It instead *removes* the compiled directory. This is generally used
  only for development of the vvp engine.

* -m<module>

  Name a VPI module that should be loaded. The vvp engine looks for the named
  module in the module search path, which includes the compiled in default
  directory and directories given by "-M" flags.

  NOTE: Starting with v11.0, the VPI modules to be loaded can be specified
  when you compile your design. This allows the compiler to automatically
  determine the return types of user-defined system functions. If specified at
  compile-time, there is no need to specify them again here.

* -s

  $stop right away, in the beginning of the simulation. This kicks the
  vvp program into interactive debug mode.

* -v

  Show verbose progress while setting up or cleaning up the runtime
  engine. This also displays some performance information.

Extended Arguments
------------------

The extended arguments are available to the simulation runtime, especially
system tasks, system functions and any VPI/PLI code. Extended arguments that
start with a "+" character are left for use by the user via the $plus$flag and
$plus$value functions.

VCD/FST/LXT Arguments
^^^^^^^^^^^^^^^^^^^^^

If not otherwise specified, the vvp engine will by default use VCD formats to
support the $dumpvars system task. The flags described here can alter that
behavior.

* -none/-vcd-none/-vcd-off/-fst-none

  Disable trace output. The trace output will be stubbed so that no trace file
  is created and the cost of dumping is avoided. All off these options are
  synonyms for turning of dumping.

* -fst

  Generate FST format outputs instead of VCD format waveform dumps. This is
  the preferred output format if using GTKWave for viewing waveforms.

* -lxt/-lxt2

  Generate LXT or LXT2format instead of VCD format waveform dumps. The LXT2
  format is more advanced.

* -dumpfile=<name>

  Set the default dumpfile. If unspecified, the default is "dump". This
  command line flag allows you do change it. If no suffix is specified,
  then the suffix will be chosen based on the dump type. In any case, the
  $dumpfile system task overrides this flag.

SDF Support
^^^^^^^^^^^

The Icarus Verilog support for SDF back-annotation can take some extended
arguments to control aspects of SDF support.

* -sdf-warn

  Print warnings during load of/annotation from an SDF file.

* -sdf-info

  Print interesting information about an SDF file while parsing it.

* -sdf-verbose

  Print warnings and info messages.

Environment Variables
---------------------

The vvp program pays attention to certain environment variables.

* IVERILOG_DUMPER

