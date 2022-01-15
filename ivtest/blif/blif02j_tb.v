
module main;

   reg [1:0] D0, D1;
   reg	     sel;
   wire [1:0] Q;

   test_mux DUT(.\S[1] (1'b0), .\S[0] (sel),
		.\D0[1] (D0[1]), .\D0[0] (D0[0]),
		.\D1[1] (D1[1]), .\D1[0] (D1[0]),
		.\Q[1] (Q[1]), .\Q[0] (Q[0]));

   initial begin
      D0 = 'b01;
      D1 = 'b10;
      sel = 0;
      #1 ;
      if (Q !== D0) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, Q=%b", D0, D1, sel, Q);
	 $finish;
      end

      sel = 1;
      #1 ;
      if (Q !== D1) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, Q=%b", D0, D1, sel, Q);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
