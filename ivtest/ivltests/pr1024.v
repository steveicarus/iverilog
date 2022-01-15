/*
 * Based on PR#1024
 */

module test;

   initial begin
      wait(1) ; /* This is weird, but legal. */
      $display("PASSED");
   end
endmodule // test
