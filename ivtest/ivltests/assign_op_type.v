module test();

reg signed [3:0] a;
reg        [7:0] b;
reg signed [7:0] c;
reg signed [7:0] d;
reg signed [7:0] e;

reg failed = 0;

initial begin
  a = -1;
  b = 4;
  c = 4;
  d = 4;
  e = 4;
  b += a;
  c += a;
  {d} += a;
  e[7:0] += a;
  $display("%0d", b);
  if (b !== 19) failed = 1;
  $display("%0d", c);
  if (c !==  3) failed = 1;
  $display("%0d", d);
  if (d !== 19) failed = 1;
  $display("%0d", e);
  if (e !== 19) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
