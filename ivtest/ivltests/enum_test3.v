module top;
  // This should be okay (the trimmed bits match the enum MSB).
  enum reg[3:0] {VAL1, XX1='bxxxxx} en1;
  // But these should fail because the trimmed bits do not match the enum MSB.
  enum reg[3:0] {VAL2, XX2='b0xxxx} en2;
  enum reg[3:0] {VAL3, XX3='b0xxxxx} en3;

  initial $display("FAILED");
endmodule
