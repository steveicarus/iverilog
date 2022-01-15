module connect(inout [1:0] c);

tran(c[0], c[1]);

endmodule

module top();

tri [3:0] a;

reg dir;

connect connect1(a[1:0]);
connect connect2(a[2:1]);
connect connect3(a[3:2]);

assign a[0] = dir ? 1'bz : 1'b0;
assign a[3] = dir ? 1'b1 : 1'bz;

reg pass = 1;

initial begin
  dir = 1'b0;
  #1 $display("%b", a);
  if (a !== 4'b0000) pass = 0;
  dir = 1'b1;
  #1 $display("%b", a);
  if (a !== 4'b1111) pass = 0;

  if (pass)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
