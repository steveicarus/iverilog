module test ();
  reg pass = 1'b1;
  reg d;
  real a = 0.0;
  wire real f = a;

  always @(d) force f = 0.0;

  initial begin
    // Verify the initial value.
    #1;
    if (f != 0.0) begin
      $display("Failed initial value, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify the value can change.
    #1 a = 1.0;
    if (f != 1.0) begin
      $display("Failed value change, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the force changed the value and that the CA is blocked.
    #1 d = 0;
    #1 a = 1.0;
    if (f != 0.0) begin
      $display("Failed force holding, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the release propagates the CA value.
    #1 release f;
    if (f != 1.0) begin
      $display("Failed release change, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the value can be changed after a release.
    #1 a = 0.0;
    if (f != 0.0) begin
      $display("Failed release, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
