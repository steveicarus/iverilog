module top;
  reg a, b, c, d, e;
  wand out;

  assign out = a;
  assign out = b;
  assign out = c;
  assign out = d;
  assign out = e;

  initial begin
    a = 1'b1;
    b = 1'b0;
    c = 1'b1;
    d = 1'b1;
    e = 1'b1;
    #1;
    if (out !== 1'b0) $display("FAILED: expected 1'b1, got %b", out);
    else $display("PASSED");
  end
endmodule
