module top;
  // You can't have an over range value (compile time error).
  enum bit[4:0] {some[4] = 100} val;

  initial $display("FAILED");
endmodule
