module test (
   Z,
   CO32,
   A,
   B,
   CI);

input[63:0]  A;
input[63:0]  B;
input	     CI;
output[63:0] Z;
output       CO32;

wire[31:0] Z1;
wire[31:0] Z2;

adder32 add0(
   .Z(Z[31:0]),
   .A(A[31:0]),
   .B(B[31:0]),
   .CI(CI));

carry32 car0(
   .CO (CO32),
   .A  (A[31:0]),
   .B  (B[31:0]),
   .CI (CI));

adder32 add1(
   .Z  (Z1),
   .A  (A[63:32]),
   .B  (B[63:32]),
   .CI (1'b0));

adder32 add2(
   .Z  (Z2),
   .A  (A[63:32]),
   .B  (B[63:32]),
   .CI (1'b1));

assign Z[63:32] = CO32 ? Z2 : Z1;

endmodule
module adder32 (
   Z,
   A,
   B,
   CI);

input[31:0]   A;
input[31:0]   B;
input	      CI;
output [31:0] Z;

assign Z = A + B + CI;

endmodule
module carry32 (
   CO,
   A,
   B,
   CI);

input[31:0] A;
input[31:0] B;
input	    CI;
output      CO;

wire[31:0] unused;
assign    {CO, unused} = A + B + CI;

endmodule
