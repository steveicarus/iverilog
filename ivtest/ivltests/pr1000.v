/*
 * Based on PR#1000.
 */

module foo20 ();
   /* This is reported to return the warning:
      warning: Ranges in localparam definition are not supported.
      The value is OK, and the compiler chooses a width that holds
      whatever value is there. */
   localparam [65:0] foo = 0;

   initial begin
      if ($bits(foo) != 66) begin
	 $display("FAILED -- $bits(foo) --> %0d", $bits(foo));
	 $finish;
      end

      $display("PASSED");
   end

endmodule
