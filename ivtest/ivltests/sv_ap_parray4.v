// Check that positional assignment patterns are supported for packed arrays
// when doing continuous assignments to array elements.

module test;

  wire [3:0][3:0] x[2];
  wire [1:0][3:0][3:0] y[2];

  assign x[0] = '{1'b1, 32'h2, 3.0, "TEST"};
  assign y[1] = '{'{1'b1, 1 + 1, 3.0, "TEST"},
                  '{5, 6, '{1'b0, 1 * 1, 3, 1.0}, 8}};

  final begin
    if (x[0] === 16'h1234 && y[1] == 32'h12345678) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
