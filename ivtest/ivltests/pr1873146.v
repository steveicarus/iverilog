module test (a, b);

  output [31 : 0] a;
  input b;

  buf bufd[31:0] (a, b);
   initial#1 $display("PASSED");
  specify
    specparam

    th = 0.9;

    // This incomplete set of specify cases needs to be handled.
    if (!b)
      (b *> a[0]) = (0);
  endspecify

endmodule
