// Check that out-of-bounds access on a 2-state vector dynamic array works and
// returns the correct value.

module test;

  bit [7:0] d[];
  logic [7:0] x;

  initial begin
    x = d[1];
    if (x === 8'h00) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 00000000, got %b",x);
    end
  end

endmodule
