// Regression test for GitHub issue 26
// This is invalid code and should result in a compile-time error

module tb;

wire [3:0] a, y;

test uut (.a(a), .y(y));

endmodule

module test(a, b, y);

input  [3:0] a;
output [3:0] y;

assign y = a;

endmodule
