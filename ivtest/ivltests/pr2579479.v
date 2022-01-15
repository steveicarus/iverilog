// Based on pr2579479

module main;

   supply0 gnd;
   supply1 vdd;
   wire    A,B;

   tranif1 uA(gnd, A, vdd);
   tranif1 uB(A, B, vdd);
   tranif1 uC(B, vdd, gnd);

   initial begin
      #1 $display("A=%d, B=%d", A,B);
      if ((A !== 1'b0) || (B !== 1'b0)) begin
	 $display("FAILED -- A=%b, B=%b", A, B);
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
