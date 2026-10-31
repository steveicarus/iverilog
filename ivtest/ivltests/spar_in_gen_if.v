module top;
  specparam sparam = 1;

  if (sparam) begin : err
  end

  initial $display("FAILED: Expected a specparam in a generate if to be invalid.");
endmodule
