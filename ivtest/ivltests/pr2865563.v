module foo ();

  parameter CLOCK_FREQUENCY        = 90e6;

  // CLOCK_PERIOD_BIT_WIDTH <= log2(90e6)
  // log2(90e6) = 26.423
  parameter CLOCK_PERIOD_BIT_WIDTH = 26;

  // build something big enaugh to hold CLOCK_FREQUENCY x CLOCK_PERIOD_BIT_WIDTH sums.
  parameter CP_SUM_BIT_WIDTH       = 2 * CLOCK_PERIOD_BIT_WIDTH;

  //
  // calculate a sane reset value.
  //
  wire [CLOCK_PERIOD_BIT_WIDTH-1:0] rst, rst2;

  assign rst  = {1'd1, {CP_SUM_BIT_WIDTH-1 {1'd0}}} / CLOCK_FREQUENCY;
  assign rst2 = (52'd2**(CP_SUM_BIT_WIDTH-1)) / CLOCK_FREQUENCY;

  initial
    #1 if (rst == rst2) $display("PASSED");
    else $display("FAILED");

endmodule // foo
