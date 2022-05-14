// Check that out-of-bounds access on a 2-state vector queue works and returns
// the correct value.

module test;

  bit [7:0] q[$];
  logic [7:0] x;

  initial begin
    x = q[1];
    if (x === 8'h00) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 00000000, got %b",x);
    end
  end

endmodule
