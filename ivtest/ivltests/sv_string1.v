/*
 * This is the most basic test of string variables.
 */
module main;

   string foo = "PASSED";

   initial begin
      $display(foo);
      $finish;
   end
endmodule // main
