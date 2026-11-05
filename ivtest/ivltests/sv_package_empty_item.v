// Check that empty package items are supported.

package p;
  ;
  parameter VALUE = 7;
  ;
endpackage

module test;
  initial begin
    if (p::VALUE === 7) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
