module top;
  // This should fail because XX5 is given an undefined constant (2-state).
  enum {VAL5, XX5 = 'bx} en5;

  initial $display("FAILED");
endmodule
