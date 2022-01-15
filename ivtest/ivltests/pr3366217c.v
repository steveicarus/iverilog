module top;
  // An implicit value cannot be over range (compile time error).
  enum bit[1:0] {a = 3, b , c} val;

  initial $display("FAILED");
endmodule
