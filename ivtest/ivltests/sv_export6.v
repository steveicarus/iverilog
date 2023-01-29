// Check that it is possible to implicitly import the same identifier through
// multiple paths without causing a conflict.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::x;
  export P1::x;
endpackage

module test;

  // P1::x is visible through either of the imports below. This should not
  // create a conflict since it is the same identifier.
  import P1::*;
  import P2::*;

  initial begin
    if (x == 123) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
