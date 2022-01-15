/*
 * This test program should cause the message "Hello, World" to
 * display twice. The first when the always thread runs and gets
 * stuck in the wait, and the second when the block is disabled,
 * and the alwas thread starts it over again.
 */

module main;

   always begin :restartable
      $display("Test thread runs.");
      wait (0);
      $display("FAILED: Should never get here.");
   end

   initial begin
      #10 disable restartable;
      #10 $finish(0);
   end

endmodule // main
