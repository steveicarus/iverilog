module top();

reg foo;

tri [1:0] a;

assign a[0] = foo;

tran(a[0], a[1]);

initial begin
  foo = 1'b1;
  #1 $display("%b", a);
  if (a === 2'b11)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
