// Check that it is an error to export an identifier that has not been imported.

package P1;
  integer x;
  integer y;
endpackage

package P2;
  import P1::x;
  export P1::y; // Should fail, P1::y has not been imported.
endpackage

module test;
  initial begin
    $display("FAILED");
  end
endmodule
