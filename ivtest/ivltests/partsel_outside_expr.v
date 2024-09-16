/*
 * partsel_outside_expr
 * Check that base_expr ± bit_number in a part-select does not wrap around
 * within the internal 32 bit (signed) vector address space.
 * This could incorrectly select bits from a vector when unsigned integers
 * are used in the part-select index expression.
 */

module main;

   reg [1:0] arr = 1;
   integer unsigned uoffset = -'d1;
   wire [1:0] outside_expr = arr[uoffset +: 2];

   initial begin
      if (outside_expr !== 'x) begin
	 $display("FAILED -- non-const base_expr out of bounds value %b != xx", outside_expr);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
