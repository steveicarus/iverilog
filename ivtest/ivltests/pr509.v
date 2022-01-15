/*
 * Make sure the degenerate case that a wire is linked to itself
 * is handled properly.
 */
module example;
   wire    w;
   assign   w = w;
   initial $display("PASSED");
endmodule
