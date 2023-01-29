// Check that it is possible to export an implicitly imported identifier and
// import it again from another scope.

package P1;
  integer x = 123;
endpackage

package P2;
  import P1::*;
  export P1::x;
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
