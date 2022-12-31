// Check that imported identifiers can't be accessed through hierarchical names.

package P;
  integer x;
endpackage

module M;
  import P::*;
  integer y;
  always_comb y = x;
endmodule

module test;

  M m ();

  initial begin
    integer y;
    y = m.x; // This should fail. Imported identifiers are not visible through
             // hierarchical names.
    $display("FAILED");
  end

endmodule
