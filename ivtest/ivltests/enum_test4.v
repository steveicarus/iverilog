module top;
  enum bit [3:0] {first, second, third, fourth, last = -4'sd1} my_type;
  initial $display("PASSED");
endmodule
