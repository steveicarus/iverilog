/*
 * See PR#810 in the test suite.
 */

`timescale 1 ns / 1 ps

module RR64X1_4LA1 ();

parameter I_AADR_01_DOA_01_T2      =       1.167000;
parameter I_AADR_10_DOA_01_T2      =       1.176000;
parameter taaa_d1      =       ( I_AADR_01_DOA_01_T2 > I_AADR_10_DOA_01_T2 )
? I_AADR_01_DOA_01_T2 : I_AADR_10_DOA_01_T2;
parameter I_AADR_01_DOA_10_T2      =       1.276000;
parameter I_AADR_10_DOA_10_T2      =       1.267000;
parameter taaa_d0      =       ( I_AADR_01_DOA_10_T2 > I_AADR_10_DOA_10_T2 )
? I_AADR_01_DOA_10_T2 : I_AADR_10_DOA_10_T2;
parameter taaa         =       ( taaa_d1 > taaa_d0 ) ? taaa_d1 : taaa_d0;

   initial begin
      if (taaa_d1 != I_AADR_10_DOA_01_T2) begin
	 $display("FAILED -- taaa_d1=%f", taaa_d1);
	 $finish;
      end

      if (taaa_d0 != I_AADR_01_DOA_10_T2) begin
	 $display("FAILED -- taaa_d0=%f", taaa_d0);
	 $finish;
      end

      if (taaa != taaa_d0) begin
	 $display("FAILED -- taaa=%f", taaa);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
