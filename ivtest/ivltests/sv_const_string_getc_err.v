module top;
  logic [3:0] sig;
  parameter string str = "Hello";
  parameter byte chr1 = str.getc(sig);
  parameter byte chr2 = str.getc();
  parameter byte chr3 = str.getc(1, 2);

  initial $display("FAILED: should have a compile error.");
endmodule
