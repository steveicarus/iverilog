// Check that out-of-bounds access on a 4-state vector queue works and returns
// the correct value.

module test;

  logic [7:0] q[$];
  logic [7:0] x;

  initial begin
    x = q[1];
    if (x === 8'hxx) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected xxxxxxxx, got %b", x);
    end
  end

endmodule
