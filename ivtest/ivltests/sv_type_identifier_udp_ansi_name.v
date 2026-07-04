// Check that ANSI-style UDP names can shadow visible type identifiers.

package p;
  typedef int ANSI_UDP;
  typedef int Y;
  typedef int I0;
  typedef int I1;
endpackage

import p::*;

primitive ANSI_UDP (output Y, input I0, input I1);
  table
    00 : 0;
    01 : 0;
    10 : 0;
    11 : 1;
  endtable
endprimitive

module test;
  initial begin
    $display("PASSED");
  end
endmodule
