program main;

   reg foo;

// It is NOT legal to nest modules in program blocks.
module test;
   initial $display("FAILED");
endmodule // test

endprogram // main
