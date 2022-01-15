// Regression test for GitHub issue #28 : Insufficient string escaping
// when writing vvp script.

module tb;

wire [63:0] y;

\test_str="hello" uut (y);

initial begin
  #1 $display("%s", y);
  if (y === "hello")
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

module \test_str="hello" (output [63:0] \port="y" );

assign \port="y" = "hello";

endmodule
