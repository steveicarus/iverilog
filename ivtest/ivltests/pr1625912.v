/* PR1625912 */

/*
 * Substatuting in either of the commented out lines caused VVP to fail.
 */

module top;
   integer cnt;
   real    result, win;

   initial begin
      cnt = -10;
      for (result=-10; result<=10; result=result+2) begin
	 #1 if (result != cnt) begin
	    $display("FAILED -- cnt=%0d, result=%f", cnt, result);
	 end
	 cnt = cnt + 2;
      end
      $display("PASSED");
      $finish;
   end

endmodule
