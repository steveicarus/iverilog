module pr2849783();

reg i;
wire a, b, c, d;

assign a = i;
assign b = a;

assign c = 1;
assign d = c;

reg pass;

initial begin
  i = 1;
  pass = 1;
  #1 $display("%b %b", a, b);
  if ((a !== 1) || (b !== 1)) pass = 0;
  #1 force a = 0;
  #1 $display("%b %b", a, b);
  if ((a !== 0) || (b !== 0)) pass = 0;
  #1 release a;
  #1 $display("%b %b", a, b);
  if ((a !== 1) || (b !== 1)) pass = 0;

  #1 $display("%b %b", c, d);
  if ((c !== 1) || (d !== 1)) pass = 0;
  #1 force c = 0;
  #1 $display("%b %b", c, d);
  if ((c !== 0) || (d !== 0)) pass = 0;
  #1 release c;
  #1 $display("%b %b", c, d);
  if ((c !== 1) || (d !== 1)) pass = 0;

  if (pass)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
