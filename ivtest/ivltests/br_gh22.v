// Regression test for GitHub issue 22

module bug();

reg  [1:0] a;
reg  [2:0] b;
wire [3:0] y;

assign y = {a >> {22{b}}, a << (0 <<< b)};

initial begin
  b = 7;
  a = 3;
  #1 $display("%b", y);
  if (y === 4'b0011)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
