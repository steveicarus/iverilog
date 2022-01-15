module top;
  wire real sml;
  wire real big;
  wire real prec;

  assign sml = 1e-20;
  assign big = 1e20;
  assign prec = 0.123456789;

  initial $display("big: %g, small: %g, precision: %0.11f", big, sml, prec);
endmodule
