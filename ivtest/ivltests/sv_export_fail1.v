// Check that it is an error to export an identifier from a package from which
// it has not been imported from.

package P1;
  integer x;
endpackage

package P2;
  import P1::x;
  export P1::x;
endpackage

package P3;
  import P1::x;
  export P2::x; // This should fail, even though P2::x is the same as P1::x
endpackage

module test;
  initial begin
    $display("FAILED");
  end
endmodule
