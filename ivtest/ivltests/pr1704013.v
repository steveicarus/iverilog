module top;
  reg clk = 0;
  reg [1:0] in = 2'b00;
  wire [1:0] out;

  test t1 (clk, out, in);
endmodule

module test(clk, a, b);
input clk;
output a;
input [1:0] b;

reg [1:0] a;

always @(posedge clk) begin
  a <= b;
end
endmodule
