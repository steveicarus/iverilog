// Regression test for GitHub issue 7 : Undef propagation in power operator.

module bug();

reg [3:0] a;
reg [3:0] y;

reg failed = 0;

initial begin
  a = 4'd1 / 4'd0;
  y = 4'd2 ** a;
  $display("%b", a);
  $display("%b", y);
  if (y !== 4'bxxxx)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
