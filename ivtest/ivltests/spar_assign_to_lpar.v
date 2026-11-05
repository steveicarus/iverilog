module top;
  specparam sparam = 1;
  localparam param = sparam;

  initial $display("FAILED: Expected a specparam to localparam to be invalid.");
endmodule
