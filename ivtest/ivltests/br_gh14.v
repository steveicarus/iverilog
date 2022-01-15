// Regression test for GitHub issue 14 : Bug in processing 1'b1 >= |1'bx.

module bug();

wire y = 1'b1 >= 1'bx;

initial begin
  #0 $display("%b", y);
  if (y !== 1'bx)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
