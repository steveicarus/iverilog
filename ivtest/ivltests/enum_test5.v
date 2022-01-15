module top;
  // This should be okay because the size matches.
  enum reg[3:0] {VAL1, XX1=4'bxxxx} en1;
  // But these should fail because the size is wrong.
  enum reg[3:0] {VAL2, XX2=3'bxxx} en2;
  enum reg[3:0] {VAL3, XX3=5'bxxxxx} en3;

  initial $display("FAILED");
endmodule
