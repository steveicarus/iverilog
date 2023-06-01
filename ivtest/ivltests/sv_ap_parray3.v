// Check that positional assignment patterns are supported for packed arrays
// when doing continuous assignments.

module test;

  wire [3:0][3:0] x;
  wire [1:0][3:0][3:0] y;

  assign x = '{1'b1, 32'h2, 3.0, "TEST"};
  assign y = '{'{1'b1, 1 + 1, 3.0, "TEST"},
               '{5, 6, '{1'b0, 1 * 1, 3, 1.0}, 8}};

  // Use an inital block with a delay since a final block cannot be converted to vlog95
  initial begin
    #1;
    if (x === 16'h1234 && y == 32'h12345678) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
