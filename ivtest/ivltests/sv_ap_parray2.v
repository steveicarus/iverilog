// Check that positional assigment patterns are supported for packed array
// parameters.

module test;

  localparam bit [2:0] x = '{1'b1, 2.0, 2 + 1};

  initial begin
    if (x === 3'b101) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
