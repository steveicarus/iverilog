// NOTE: The expected results for this test will depend on the order in
// which the compiler elaborates the statements.
module top;
  uwire [7:0] w;

  assign w[5:2] = 4'd0;
  assign w[1:0] = 2'd1;
  assign w[3:2] = 2'd1;
  assign w[5:4] = 2'd1;
  assign w[7:6] = 2'd1;

  initial $display("Failed: this should be a compile time error!");
endmodule
