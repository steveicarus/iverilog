/*
 * partsel_outside_const
 * Check that base_const ± bit_number in a part-select does not wrap around
 * within the internal 32 bit (signed) vector address space.
 * This could incorrectly select bits from a vector when unsized literal
 * constant numbers are used in the part-select index expression.
 */

module main;

   reg [1:0] arr = 1;
   wire [1:0] outside_const = arr[-'d1 +: 2];

   initial begin
      if (outside_const !== 'x) begin
	 $display("FAILED -- const base_expr out of bounds value %b != xx", outside_const);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
