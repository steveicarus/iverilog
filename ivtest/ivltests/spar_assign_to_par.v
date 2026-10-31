module top;
  specparam sparam = 1;
  parameter param = sparam;

  initial $display("FAILED: Expected a specparam to parameter to be invalid.");
endmodule
