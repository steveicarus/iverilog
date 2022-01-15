module m1(output reg [7:0] x);
endmodule

module m2(input [3:0] y);
endmodule

module tb;
  wire [3:0] y;
  m1 foo({4'hx, y});
  m2 bar(.y(y));
endmodule
