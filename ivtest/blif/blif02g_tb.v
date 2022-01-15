
module main;

   localparam WID = 4;
   reg [WID:0] X;
   wire        q_and, q_or, q_xor, q_nand, q_nor, q_xnor;

   test_logic DUT(.\A[3] (X[3]), .\A[2] (X[2]), .\A[1] (X[1]), .\A[0] (X[0]),
		  .q_and(q_and), .q_or(q_or), .q_xor(q_xor),
		  .q_nand(q_nand), .q_nor(q_nor), .q_xnor(q_xnor));

   initial begin
      for (X = 0 ; X < 16 ; X = X+1) begin
	 #1 /* Let gates settle. */;
	 if (q_and !== & X[WID-1:0]) begin
	    $display("FAILED -- q_and=%b, X=%b", q_and, X[WID-1:0]);
	    $finish;
	 end
	 if (q_or  !== | X[WID-1:0]) begin
	    $display("FAILED -- q_or=%b, X=%b", q_or, X[WID-1:0]);
	    $finish;
	 end
	 if (q_xor !== ^ X[WID-1:0]) begin
	    $display("FAILED -- q_xor=%b, X=%b", q_xor, X[WID-1:0]);
	    $finish;
	 end
	 if (q_nand !== ~& X[WID-1:0]) begin
	    $display("FAILED -- q_nand=%b, X=%b", q_nand, X[WID-1:0]);
	    $finish;
	 end
	 if (q_nor  !== ~| X[WID-1:0]) begin
	    $display("FAILED -- q_nor=%b, X=%b", q_nor, X[WID-1:0]);
	    $finish;
	 end
	 if (q_xnor !== ~^ X[WID-1:0]) begin
	    $display("FAILED -- q_xnor=%b, X=%b", q_xnor, X[WID-1:0]);
	    $finish;
	 end

      end
      $display("PASSED");
   end

endmodule // main
