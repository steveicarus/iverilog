module test ();
  reg pass = 1'b1;
  reg d;
  real f = 0.0;

  always @(d) assign f = 0;

  initial begin
    // Verify the initial value.
    #1;
    if (f != 0.0) begin
      $display("Failed initial value, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify the value can change.
    #1 f = 1.0;
    if (f != 1.0) begin
      $display("Failed value change, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the assign changed the value and that a normal assign
    // is blocked.
    #1 d = 0;
    #1 f = 1.0;
    if (f != 0.0) begin
      $display("Failed assign holding, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the release holds the previous value.
    #1 deassign f;
    if (f != 0.0) begin
      $display("Failed release holding, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the value can be changed after a release.
    #1 f = 1.0;
    if (f != 1.0) begin
      $display("Failed release, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
