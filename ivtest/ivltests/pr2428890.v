module top;
  lower #(0, 0, 1) dut();
endmodule

module lower;
  parameter one = 0;                  // This should be 'sd0
  parameter two = 0;                  // This should be 'sd0
  parameter three = 0;                // This should be 'sd1
  parameter local1 = one - two;       // This should be 'sd0
  // This line is not working correctly.
  // The 1 is not considered signed!
  // local1 + 1 is giving 'd1 not 'sd1.
  parameter local2 = local1+1-three;  // This should be 'sd0
  // Even this fails.
//  parameter local2 = local1+'sd1-three;  // This should be 'sd0

  initial begin
    // This should be 2 < -1.
    if (2 < local2-1) $display("FAILED");
    else $display("PASSED");
  end
endmodule
