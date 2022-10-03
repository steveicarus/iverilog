// Check that implicitly imported identifiers can be exported through a package
// wildcard export.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::*;
  export P1::*;
  integer y = x; // Creates an import for P1::x
endpackage

module test;

  import P2::x;

  initial begin
    if (x == 123) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
