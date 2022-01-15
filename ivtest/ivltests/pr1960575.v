module test;
   initial
     $write("expected x; got %0b\n", 1'b0 ^ 1'bz);
endmodule
