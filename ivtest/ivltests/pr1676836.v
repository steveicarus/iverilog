`define FAIL

module top;
  reg[7:0] pattern;

  initial begin
    // Mask off the MSB and the two lower bits.
    pattern = ~0 ^ 2'b11;
    pattern[7] = 0;
`ifndef FAIL
    #0;
`endif
    if ((8'b01111110&pattern) == (8'b11111101&pattern)) $display("PASSED");
    else $display("Fail: %b", pattern);
  end
endmodule
