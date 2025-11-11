module test;

reg [1:0] a;
reg       b;
reg       c;
reg       d;

always_latch begin
  if (a[1])
    c = 1;
  else if (a[0])
    c = 0;
end

always_latch begin
  if (b)
    d = 1;
  else if (a[0])
    d = 0;
end

reg failed = 0;

initial begin
  $monitor("%0t : %b %b : %b %b", $time, a, b, c, d);
  a[0] = 1'b1;
  #1 if (c !== 1'b0 && d !== 1'b0) failed = 1;
  a[1] = 1'b1;
  #1 if (c !== 1'b1 && d !== 1'b0) failed = 1;
  b = 1'b1;
  #1 if (c !== 1'b1 && d !== 1'b1) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
