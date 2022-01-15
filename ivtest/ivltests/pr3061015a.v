module top;
  // This should be a compilation error.
  parameter PARAMB = PARAMB + 6;

  initial $display("FAILED");
endmodule
