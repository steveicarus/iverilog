module main;

   generate
      genvar i;
      for( i=0; i<4; i=i+2 )
	begin : U
	   reg [1:0] a;
	   initial begin : V
	      a = 2'b0;
	      #10;
	      a = i;
	   end
	end
   endgenerate

   initial begin
      #5 ;
      if (U[0].V.a !== 2'd0) begin
	 $display("FAILED -- U[0].V.a = %d", U[0].V.a);
	 $finish;
      end
      if (U[2].V.a !== 2'd0) begin
	 $display("FAILED -- U[2].V.a = %d", U[2].V.a);
	 $finish;
      end
      #10 ;
      if (U[0].V.a !== 2'd0) begin
	 $display("FAILED -- U[0].V.a = %d", U[0].V.a);
	 $finish;
      end
      if (U[2].V.a !== 2'd2) begin
	 $display("FAILED -- U[2].V.a = %d", U[2].V.a);
	 $finish;
      end
      $display("PASSED");
      $finish;
   end

endmodule
