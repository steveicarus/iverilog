// Check that a module name can have the same spelling as a visible typedef.

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
