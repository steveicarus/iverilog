// Regression test for GitHub issue 8 : Signedness handling in binary
// bitwise operations of constants.

module bug();

localparam value1 = 4'sb1010 | 4'sb0000;
localparam value2 = 4'sb1010 + 4'sb0000;
localparam value3 = ~4'sb0101;
localparam value4 = -4'sb0101;

reg signed [4:0] result;

reg failed = 0;

initial begin
  result = value1;
  $display("%b", result);
  if (result !== 5'b11010) failed = 1;
  result = value2;
  $display("%b", result);
  if (result !== 5'b11010) failed = 1;
  result = value3;
  $display("%b", result);
  if (result !== 5'b11010) failed = 1;
  result = value4;
  $display("%b", result);
  if (result !== 5'b11011) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
