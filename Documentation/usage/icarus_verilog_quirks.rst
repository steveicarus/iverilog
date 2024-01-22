
Icarus Verilog Quirks
=====================

This is a list of known quirks that are presented by Icarus Verilog. The idea
of this chapter is to call out ways that Icarus Verilog differs from the
standard, or from other implementations.

This is NOT AN EXHAUSTIVE LIST. If something is missing from this list, let us
know and we can add documentation.

System Tasks - Unique to Icarus Verilog
---------------------------------------

These are system tasks that are unique to Icarus Verilog. Don't use any of
these if you want to keep your code portable across other Verilog compilers.

$readmempath
^^^^^^^^^^^^
The "$readmemb" and "$readmemh" system tasks read text files that contain data
values to populate memories. Normally, those files are found in a current work
directory. The "$readmempath()" system task can be used to create a search
path for those files. For example:

.. code-block:: verilog

  reg [7:0] mem [0:7];
  initial begin
    $readmemh("datafile.txt", mem);
  end

This assumes that the "datafile.txt" is in the current working directory where
the vvp command is running. But with the "$readmempath", one can specify a
search path:

.. code-block:: verilog

  reg [7:0] mem [0:7];
  initial begin
    $readmempath(".:alternative:/global/defaults");
    $readmemh("datafile.txt", mem);
  end

In this example, the "datafile.txt" is searched for in each of the directories
in the above list (separated by ":" characters). The first located instance
is the one that is used. So for example, if "./datafile.txt" exists, then it
is read instead of "/global/defaults/datafile.txt" even if the latter exists.
