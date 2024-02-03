// NOTE: The expected results for this test will depend on the order in
// which the compiler elaborates the statements.
module top;
  uwire [3:0] w[3:0];

  assign w[1] = 4'd0;
  assign w[2] = 4'd1;
  assign w[0] = 4'd2;
  assign w[1] = 4'd3;
  assign w[2] = 4'd4;
  assign w[3] = 4'd5;

  initial $display("Failed: this should be a compile time error!");
endmodule
