/*
  I seem to have found a problem with the $monitor task/events.
  (this is probably related to bug 399)
  The problem only seems to arise in vvp mode and not in vvm.
  Problem: $monitor seems to lose both the first and last time steps.

  A complete copy of the run follows with source appended at the end.
  (The correct output from vvm is shown in the last run)
  This file compiles and produces the problem.

  jungle_geo@hotmail.com

*/

/*

bubba> uname -a
Linux bubba 2.2.15-4mdk #1 Wed May 10 15:31:30 CEST 2000 i686 unknown

bubba> iverilog -V
Icarus Verilog version 0.6
Copyright 1998-2002 Stephen Williams
$Name:  $

bubba> iverilog -Wall -tvvp stim.v
bubba> a.out
Time = 1 a = 1

bubba> iverilog -Wall -tvvm stim.v
bubba> a.out
Time = 0 a = 0
Time = 1 a = 1
Time = 2 a = 0

*/

// -------------------------------------------------------------------------stim
module stim;
  reg  a;

  initial begin
    a = 0;
    #1 a = 1;
    #1 a = 0;
  end

  initial begin
    $monitor("Time = %0d a = %b", $time, a);
  end

endmodule
