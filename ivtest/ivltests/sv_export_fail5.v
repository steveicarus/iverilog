// Check that it is an error to export an identifier that is importable through
// a wildcard import if it creates a conflict with a local identifier, even if
// the local identifier is declared after the export.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::*;
  export P1::x; // This should fail, P1::x can not be imported into this scope
                // since the there is a local symbol with the same name. Even if
                // it is declared after the export.
  integer x = 456;
endpackage

module test;

  import P2::x;

  initial begin
    $display("FAILED");
  end

endmodule
