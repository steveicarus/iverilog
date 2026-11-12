// Check compound assignments to part selects with negative constant offsets.

module test;

  reg [7:0] value;

  function [7:0] calc(input [7:0] initial_value);
    calc = initial_value;
    calc[1:-2] ^= 4'b1100;
  endfunction

  initial begin
    value = 8'h30;
    value[1:-2] ^= 4'b1100;
    if (value !== 8'h33 || calc(8'h30) !== 8'h33) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
