// Check that an identifier importable through a wildcard import is not exported
// through a wildcard export if the identifier has not been referenced in
// package.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::*;
  export *::*;
endpackage

module test;

  import P2::x; // This should fail, P1::x is not referenced in P2 and hence not
                // exportable through P2

  initial begin
    $display("FAILED");
  end

endmodule
