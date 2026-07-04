// Check that old-style UDP names can shadow visible type identifiers.

package p;
  typedef int OLD_UDP;
  typedef int Q;
  typedef int A;
  typedef int B;
endpackage

import p::*;

primitive OLD_UDP (Q, A, B);
  output reg Q;
  input A, B;
  initial Q = 0;

  table
    00 : ? : 0;
    01 : ? : 0;
    10 : ? : 0;
    11 : ? : 1;
  endtable
endprimitive

module test;
  initial begin
    $display("PASSED");
  end
endmodule
