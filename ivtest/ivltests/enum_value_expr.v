module test;
   parameter    PVALUE = 12;
   localparam   LVALUE = 88;
   enum { P[5] = PVALUE, Q, S[3] = LVALUE } par_enum;

   initial begin
      // constants elaborated from parameter
      if (P0 != PVALUE+0 || P1 != PVALUE+1 || P2 != PVALUE+2 || P3 != PVALUE+3 || P4 != PVALUE + 4 || Q != PVALUE+5)
	begin
	   $display ("FAILED - Initialised values P in par_enum were not elaborated properly");
	   $finish;
	end
      // constants elaborated from localparam
      if (S0 != LVALUE+0 || S1 != LVALUE+1 || S2 != LVALUE+2)
	begin
	   $display ("FAILED - Initialised values S in par_enum were not elaborated properly");
	   $finish;
	end
      #1;
      // checking num method
      if ( par_enum.num != 9)
	begin
	   $display ("FAILED - The num method does not report as expected");
	   $finish;
	end
      $display ("PASSED");
   end

endmodule
