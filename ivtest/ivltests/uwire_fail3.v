// NOTE: The expected results for this test will depend on the order in
// which the compiler elaborates the statements.
module top;
  uwire [3:0] w;

  assign w[1] = 1'b0;
  assign w[2] = 1'b1;
  assign w[0] = 1'b1;
  assign w[1] = 1'b1;
  assign w[2] = 1'b1;
  assign w[3] = 1'b1;

  initial $display("Failed: this should be a compile time error!");
endmodule
