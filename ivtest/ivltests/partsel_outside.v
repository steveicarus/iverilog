/*
 * partsel_outside
 * Check that base_expr ± bit_number in a part-select does not wrap around
 * within the internal 32 bit (signed) vector address space.
 * This could incorrectly select bits from a vector when unsized numbers are
 * used in the part-select index expression.
 */

module main;

   reg [1:0] arr = 1;
   reg [31:0] uoffset = -32'd1;
   wire [1:0] outside_const = arr[-32'd1 +: 2];
   wire [1:0] outside_expr = arr[uoffset +: 2];

   initial begin
      #1 if (outside_const !== 'x) begin
	 $display("FAILED -- const base_expr out of bounds value %b != xx", outside_const);
	 $finish;
      end

      #2 if (outside_expr !== 'x) begin
	 $display("FAILED -- non-const base_expr out of bounds value %b != xx", outside_expr);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
