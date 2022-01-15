// Adapted from test case supplied in github issue #6

module bug();

reg  a;
wire y;

assign y = |(-a);

reg failed = 0;

initial begin
  a = 0;
  #1 $display("a = %b y = %b", a, y);
  if (y !== 0) failed = 1;
  a = 1;
  #1 $display("a = %b y = %b", a, y);
  if (y !== 1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
