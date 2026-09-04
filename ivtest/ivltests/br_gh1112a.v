module top;
  // An undefined value cannot be passed to $bits().
  localparam width = $bits(value);
  initial $display("FAILED: elaborated a width of %0d", width);
endmodule
