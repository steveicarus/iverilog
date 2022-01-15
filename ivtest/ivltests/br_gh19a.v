// Regression test for GitHub issue 19 : Icarus only using the lowest 32
// bits of right shift operand (run-time test)

module bug();

reg a;
reg y;

initial begin
  a = 1;
  y = 1 >> {a, 64'b0};
  $display("%b", y);
  if (y === 1'b0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
