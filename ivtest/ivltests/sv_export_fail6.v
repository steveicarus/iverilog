// Check that an error is reported if trying to export an identifier that is
// declared outside of a package

integer x = 123;

package P1;
endpackage

package P2;
  import P1::*;
  export P1::x; // This should fail. x is visible in P1, but not declared in P1
endpackage

module test;

  import P2::x;

  initial begin
    $display("FAILED");
  end

endmodule
