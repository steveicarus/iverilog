module top;
  string str = "Hello";
  byte chr1 = str.getc();
  byte chr2 = str.getc(1, 2);

  initial $display("FAILED: should have a compile error.");
endmodule
