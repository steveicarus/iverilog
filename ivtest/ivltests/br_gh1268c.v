module top;
  reg outl, outr, ctl;

  // A variable can never be connected to a bidirectional port
  tranif0(outl, outr, ctl);

  initial $display("Failed should not compile!");
endmodule
