
module main;

   reg [1:0] D0, D1;
   reg [1:0] sel;
   wire [1:0] Q, R;

   test_mux DUT(.\S[1] (sel[1]), .\S[0] (sel[0]),
		.\D0[1] (D0[1]), .\D0[0] (D0[0]),
		.\D1[1] (D1[1]), .\D1[0] (D1[0]),
		.\Q[1] (Q[1]), .\Q[0] (Q[0]),
		.\R[1] (R[1]), .\R[0] (R[0]));

   initial begin
      D0 = 'b01;
      D1 = 'b10;
      sel = 0;
      #1 ;
      if (Q !== D0) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, Q=%b", D0, D1, sel, Q);
	 $finish;
      end
      if (R !== D0) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, R=%b", D0, D1, sel, R);
	 $finish;
      end

      sel = 1;
      #1 ;
      if (Q !== D1) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, Q=%b", D0, D1, sel, Q);
	 $finish;
      end
      if (R !== D1) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, R=%b", D0, D1, sel, R);
	 $finish;
      end

      sel = 2;
      #1 ;
      if (Q !== 'b00) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, Q=%b", D0, D1, sel, Q);
	 $finish;
      end
      if (R !== 'b00) begin
	 $display("FAILED -- D0=%b, D1=%b, S=%b, R=%b", D0, D1, sel, R);
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
