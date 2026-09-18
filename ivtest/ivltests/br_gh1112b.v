module top;
  // A scope cannot be passed to $bits().
  localparam width = $bits(top);
  initial $display("FAILED: elaborated a width of %0d", width);
endmodule
