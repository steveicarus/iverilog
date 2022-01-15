// Regression test for GitHub issue 19 : Icarus only using the lowest 32
// bits of right shift operand (run-time test)

module bug();

wire a = 1;
wire y = 1 >> {a, 64'b0};

initial begin
  #0 $display("%b", y);
  if (y === 1'b0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
