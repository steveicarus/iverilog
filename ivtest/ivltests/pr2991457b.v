/*
 * This is interesting, and not completely intuitive. In the code
 * below, the variable "tmp" is assigned the value 4'bxxxx, then
 * compated with the unsized literal 'hx. Since 'hx is unsized, it
 * is padded to the width of an integer, and the padding is done
 * by extending the 'bx. But in the comparison, the unsigned "tmp"
 * is ZERO extended. Therefore, "tmp" and 'hx are NOT equal.
 */
module main;

   reg [3:0] tmp;

   initial begin
      tmp = 'hx;
      if (tmp !== 'hx) begin
	 $display("PASSED");
	 $finish;
      end
      $display("FAILED");
   end

endmodule // main
