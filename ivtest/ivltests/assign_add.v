/*
 * assign_sum
 * Demonstrate continuous assign of a sum.
 */

module main;

   reg [8:0] A, B;
   wire [9:0] sum = A + B;

   initial begin
      A = 51;
      B = 39;
      #1 $display("%b + %b = %b", A, B, sum);
      if (sum !== 90) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
