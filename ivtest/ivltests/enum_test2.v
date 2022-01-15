module top;
  // This will compile.
  enum integer {IDLE, UNKNOWN='bx} en1;
  // This is failing to compile.
  enum integer {VAL, XX=32'bx} en2;

  initial $display("PASSED");
endmodule
