/*
 * This demonstrates that strings can be used as
 * constructed formats in $display et al.
 */
module main;

   string foo;
   string bar;

   initial begin
      foo = "%0s";
      bar = "PASSED";

      $display(foo, bar);
      $finish;
   end
endmodule // main
