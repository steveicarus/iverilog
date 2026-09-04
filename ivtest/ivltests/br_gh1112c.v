module top;
  // A real value cannot be passed to $bits().
  localparam width = $bits(1.0);
  initial $display("FAILED: elaborated a width of %0d", width);
endmodule
