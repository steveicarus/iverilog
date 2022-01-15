
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
module add1(input A, input B, input Cin, output Q, output Cout);

   assign Q = A ^ B ^ Cin;
   assign Cout = A&B | A&Cin | B&Cin;

endmodule // hadd

`ifdef TEST_BENCH
module main;

   parameter WID = 4;
   reg [WID-1:0] A, B;
   wire [WID:0]  Q;

   addN #(.WID(WID)) usum (.A(A), .B(B), .Q(Q));

   int		 adx;
   int		 bdx;
   initial begin
      for (bdx = 0 ; bdx[WID]==0 ; bdx = bdx+1) begin
	 for (adx = 0 ; adx[WID]==0 ; adx = adx+1) begin
	    A <= adx[WID-1:0];
	    B <= bdx[WID-1:0];
	    #1 if (Q !== (adx+bdx)) begin
	       $display("FAILED -- A=%b, B=%b, Q=%b", A, B, Q);
	       $finish;
	    end
	 end
      end
      $display("PASSED");
   end

endmodule // main
`endif
