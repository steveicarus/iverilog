module test;

reg [5:0] result;

reg failed = 0;

initial begin
  result = $clog2(1'sd1);
  $display("%0d", result);
  if (result !== 0) failed = 1;

  result = $clog2(-'sd1);
  $display("%0d", result);
  if (result !== 32) failed = 1;

  result = $clog2(32'h80000000);
  $display("%0d", result);
  if (result !== 31) failed = 1;

  // $clog2 treats its input as an unsigned integer vector so
  // the next two tests should produce the same result as the
  // previous one.

  result = $clog2(32'sh80000000);
  $display("%0d", result);
  if (result !== 31) failed = 1;

  result = $clog2(2**31);
  $display("%0d", result);
  if (result !== 31) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
