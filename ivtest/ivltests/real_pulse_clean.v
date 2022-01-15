`timescale 1us/100ns

module top;
  reg pass = 1'b1;
  real in1, in2;
  wire real mult;

  /* A simple multiplier. */
  assign #0.2 mult = in1*in2;

  initial begin
    #1;
    in1 = 1.0;
    in2 = 2.0;
    #1;
    if (mult != 2.0) begin
      $display("FAILED (1): expected 2.0 got %g", mult);
      pass = 0;
    end
    #1;
    in1 = 2.0;
    in2 = 1.0;
    #1;
    if (mult != 2.0) begin
      $display("FAILED (2): expected 2.0 got %g", mult);
      pass = 0;
    end
    #1;
    in1 = 1.0;
    in2 = 2.0;
    #1;
    if (mult != 2.0) begin
      $display("FAILED (3): expected 2.0 got %g", mult);
      pass = 0;
    end

    if (pass) $display("PASSED");
  end
endmodule
