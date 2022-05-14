// Check that out-of-bounds access on a 4-state vector dynamic array works and
// returns the correct value.

module test;

  logic [7:0] d[];
  logic [7:0] x;

  initial begin
    x = d[1];
    if (x === 8'hxx) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected xxxxxxxx, got %b", x);
    end
  end

endmodule
