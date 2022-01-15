
/*
 * Generate a combinational adder of any width. The width parameter can
 * be any integer value >0. The A and B inputs have WID bits, and the Q
 * output has WID+1 bits to include the overflow.
 */
module addN
  #(parameter WID = 4)
   (input wire [WID-1:0] A,
    input wire [WID-1:0] B,
    output wire [WID:0]  Q
    /* */);

   wire [WID-1:0]	Cout;

   /* The least significant slice has no Cin */
   add1 U0 (.A(A[0]), .B(B[0]), .Cin(1'b0), .Q(Q[0]), .Cout(Cout[0]));

   /* Generate all the remaining slices */
   genvar i;
   for (i = 1 ; i < WID ; i = i+1) begin : U
      add1 Un (.A(A[i]), .B(B[i]), .Cin(Cout[i-1]), .Q(Q[i]), .Cout(Cout[i]));
   end

   assign Q[WID] = Cout[WID-1];

endmodule // add

/*
 * This is a single-bit combinational adder used by the addH module
 * above.
 */
module add1(input A, input B, input Cin, output reg Q, output reg Cout);

   always @* begin
      Q = A ^ B ^ Cin;
      Cout = A&B | A&Cin | B&Cin;
   end

endmodule // hadd
