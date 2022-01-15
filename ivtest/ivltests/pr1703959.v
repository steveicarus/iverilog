// pr1703959

module test();
  ma ia (.IO(b), .ZI(c));
  mb ib( .b({b}), .c({c}));
//  mb ib( .b(b), .c(c));

  initial #10 $display("PASSED");
endmodule

module ma(ZI,IO);
  inout ZI;
  inout IO;

  pmos (ZI, IO, 1'b0);
//  pmos (IO, ZI, 1'b0);
endmodule

module mb ( b, c);
//  input b;
  output b;
  input c;
//  inout b;
//  inout c;

endmodule // mb
