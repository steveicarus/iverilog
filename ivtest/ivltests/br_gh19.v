// Regression test for GitHub issue 19 : Icarus only using the lowest 32
// bits of right shift operand.

module bug();

wire [3:0] y = 4'b1 << 33'h100000000;

initial begin
  #0 $display("%b", y);
  if (y === 4'b0000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
