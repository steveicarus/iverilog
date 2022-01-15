// This trivial program is NOT valid. "always" blocks are not
// valid in program blocks.

program main ();

   initial $display("Hello, World.");
   always #1 $finish;
   final $display("FAILED");

endprogram : main
