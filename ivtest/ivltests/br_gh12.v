// Regression test for GitHub issue 12 : Ternary lval-rval width mismatch.

module bug();

wire [1:0] a;
wire [1:0] y;

assign a = 2'b10;

assign y = 'bx ? 2'b00 : a;

reg failed = 0;

initial begin
  #0;

  $display("%b", y);
  if (y !== 2'bx0) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
