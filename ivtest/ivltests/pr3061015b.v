module top;
  // This should be a compilation error.
  parameter PARAMB = PARAMA;
  parameter PARAMA = PARAMB;

  initial $display("FAILED");
endmodule
