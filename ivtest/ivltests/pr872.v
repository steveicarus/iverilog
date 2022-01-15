/*
 * Based on PR#872
 */

module main;

   wire test;

   foo u1(test);

   initial begin
      #1 if (test !== 0) begin
	 $display("FAILED -- foo.bar == %b (%b)", test, u1.bar);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

/*
 * This the this syntax I'm testing.
 */
module foo (output reg bar = 0);
endmodule
