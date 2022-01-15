module top;
  reg [7:0] vec = 8'b10100101;
  reg passed = 1;

  function [3:0] copy;
    input [3:0] in;
    copy = in;
  endfunction

  initial begin
    if (copy(vec>>4) != 4'b1010) begin
      passed = 0;
      $display("Failed!");
    end
    if (passed) $display("PASSED");
  end
endmodule
