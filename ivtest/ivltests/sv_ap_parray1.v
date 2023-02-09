// Check that positional assigment patterns are supported for packed arrays.

module test;

  bit [3:0][3:0] x = '{1'b1, 1 + 1, 3.0, "TEST"};

  // Check nested assignment pattern
  bit [1:0][3:0][3:0] y = '{'{1'b1, 1 + 1, 3.0, "TEST"},
                            '{5, 6, '{1'b0, 1 * 1, 3, 1.0}, 8}};

  initial begin
    if (x === 16'h1234 && y == 32'h12345678) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
