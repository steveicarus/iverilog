module main;

   reg signed [5:0] GAIN = 2;
   reg signed [23:0] iir = -8;

   wire signed [23:0] iir_s1 = iir >>> 2;
   wire signed [23:0] iir_s2 = iir >>> GAIN;

   initial begin
      #1 /* Wait for inputs values to settle. */ ;

      if (iir_s1 !== -24'sd2) begin
	 $display("FAILED -- s1 = %d (%h)", iir_s1, iir_s1);
	 $finish;
      end

      if (iir_s2 !== -24'sd2) begin
	 $display("FAILED -- s2 = %d (%h)", iir_s2, iir_s2);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
