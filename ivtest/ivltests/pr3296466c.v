module top();

reg foo;
reg bar;

tri [1:0] a;
tri [1:0] b;

assign a[0] = foo;
assign b[1] = bar;

tran t[1:0](a, b);

initial begin
  foo = 1'b1;
  bar = 1'b0;
  #1 $display("%b %b", a, b);
  if ((a === 2'b01) && (b === 2'b01))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
