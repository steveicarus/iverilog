// Check that it is an error to export an identifier that is importable through
// a wildcard import if it creates a conflict with a local identifier.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::*;
  integer x = 456;
  export P1::x; // This should fail, P1::x can not be imported into this scope
                // since the name already exists.
endpackage

module test;

  import P2::x;

  initial begin
    $display("FAILED");
  end

endmodule
