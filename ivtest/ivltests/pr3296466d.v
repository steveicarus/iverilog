module top();

reg foo;

tri [1:0] a;
tri [1:0] b;
tri [3:0] c;

assign a[0] = foo;

tran t1(a[0], a[1]);
tran t2(b[0], b[1]);

tran t3[1:0](a, c[1:0]);
tran t4[1:0](b, c[3:2]);

tran t5(c[1], c[3]);

initial begin
  foo = 1'b1;
  #1 $display("%b %b %b", a, b, c);
  if ((a === 2'b11) && (b === 2'b11) && (c === 4'b1111))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
