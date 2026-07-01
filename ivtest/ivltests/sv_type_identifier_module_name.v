// Check that a module name can shadow a visible type identifier.

package p;
  typedef int M;
endpackage

import p::*;

module M;
  initial begin
    $display("PASSED");
  end
endmodule

module test;
  M i();
endmodule
