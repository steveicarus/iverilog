// Check binary NAND/NOR with four-state operands in constant functions.

module test;

  function [31:0] calculate(input [15:0] left, input [15:0] right);
    calculate = {left ~& right, left ~| right};
  endfunction

  localparam [31:0] RESULT = calculate(16'b0000_1111_xxxx_zzzz,
                                     16'b01xz_01xz_01xz_01xz);

  initial begin
    if (RESULT !== 32'b1111_10xx_1xxx_1xxx_10xx_0000_x0xx_x0xx) begin
      $display("FAILED: unexpected NAND/NOR results %b", RESULT);
    end else begin
      $display("PASSED");
    end
  end

endmodule
