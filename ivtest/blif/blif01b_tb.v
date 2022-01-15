
module main;

   reg [2:0] X;
   wire      q_nand, q_nor, q_xnor, q_not;

   test_logic DUT(.A(X[0]), .B(X[1]), .q_nand(q_nand), .q_nor(q_nor),
		  .q_xnor(q_xnor), .q_not(q_not));

   initial begin
      for (X = 0 ; X < 4 ; X = X+1) begin
	 #1 /* Let gates settle. */;
	 if (q_nand !== (X[0] ~& X[1])) begin
	    $display("FAILED -- q_nand=%b, X=%b", q_nand, X[1:0]);
	    $finish;
	 end
	 if (q_nor !== (X[0] ~| X[1])) begin
	    $display("FAILED -- q_nor=%b, X=%b", q_nor, X[1:0]);
	    $finish;
	 end
	 if (q_xnor !== (X[0] ~^ X[1])) begin
	    $display("FAILED -- q_xnor=%b, X=%b", q_xnor, X[1:0]);
	    $finish;
	 end
	 if (q_not !== (~X[0])) begin
	    $display("FAILED -- q_not=%b, X=%b", q_not, X[0]);
	    $finish;
	 end

      end
      $display("PASSED");
   end

endmodule // main
