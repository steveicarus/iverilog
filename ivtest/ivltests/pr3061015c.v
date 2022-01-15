module top;
  // This should be a compilation error.
  parameter real PARAMB = PARAMB + 1.0;

  initial $display("FAILED");
endmodule
