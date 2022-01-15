module top;
  uwire two;

  assign two = 1'b1;
  assign two = 1'b0;

  initial $display("Failed: this should be a compile time error!");
endmodule
