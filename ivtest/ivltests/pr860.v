/*
 * This is based on bug report PR#860.
 */
module stam () ;

   parameter WIDTH_IN = 8;
   parameter ABS = 0;
   parameter M = (!ABS) ? 7 : WIDTH_IN;

   parameter AFTER_DC_WIDTH = M;

   initial begin
      $display("AFTER_DC_WIDTH=%d", AFTER_DC_WIDTH);
      if (AFTER_DC_WIDTH !== 7) begin
	 $display("FAILED -- M = %d", M);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // stam
