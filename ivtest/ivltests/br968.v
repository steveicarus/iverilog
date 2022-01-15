module top;
  reg passed;
  reg [7:0] val;
  reg signed [7:0] sval;
  real rval;

  initial begin
    passed = 1'b1;
    val = 8'hff;
    sval = 8'hff;
    /* Check a constant unsigned value cast to signed. */
    rval = $itor($signed(8'hff));
    if (rval != -1.0) begin
      $display("Failed unsigned constant cast to signed conversion, ",
               "expected -1.0, got %g.", rval);
      passed = 1'b0;
    end
    /* Check an unsigned variable cast to signed. */
    rval = $itor($signed(val));
    if (rval != -1.0) begin
      $display("Failed unsigned variable cast to signed conversion, ",
               "expected -1.0, got %g.", rval);
      passed = 1'b0;
    end
    /* Check a constant signed value. */
    rval = $itor(8'shff);
    if (rval != -1.0) begin
      $display("Failed signed constant conversion, ",
               "expected -1.0, got %g.", rval);
      passed = 1'b0;
    end
    /* Check a variable signed value. */
    rval = $itor(sval);
    if (rval != -1.0) begin
      $display("Failed signed variable conversion, ",
               "expected -1.0, got %g.", rval);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
