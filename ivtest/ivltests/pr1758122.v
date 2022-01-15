`begin_keywords "1364-2001-noconfig"
// Copyright 2007, Martin Whitaker.
// This code may be freely copied for any purpose.

module gen_param_test();

localparam	W = 3;
localparam	D = 3;

reg  [W-1:0]	A[1:D];
reg  [W-1:0]	B[1:D];
wire [W-1:0]	Y[1:D];

generate
  genvar	j;

  for (j = 1; j <= D; j = j + 1) begin:sum
    adder #(W) instance(A[j], B[j], Y[j]);
  end
endgenerate

integer		i;

initial begin
  for (i = 1; i <= D; i = i + 1) begin
    A[i] = i - 1;
    B[i] = i + 1;
  end
  #1;
  for (i = 1; i <= D; i = i + 1) begin
    $display("%d + %d = %d", A[i-1], B[i-1], Y[i-1]);
  end
end

endmodule


module adder #(parameter W = 1) (

input  wire [W-1:0]	A,
input  wire [W-1:0]	B,
output wire [W-1:0]	Y

);

assign Y = A + B;

endmodule
`end_keywords
