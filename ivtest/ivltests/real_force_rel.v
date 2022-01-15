module test ();
  reg pass = 1'b1;
  reg d;
  real f = 0.0, z = 0.0, y = 1.0;

  always @(d) force f = z;

  initial begin
    // Verify the initial value.
    #1;
    if (f != 0.0) begin
      $display("Failed initial value, expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the force changed the value and that a normal assign
    // is blocked.
    #1 d = 0;
    #1 f = 1.0;
    if (f != 0.0) begin
      $display("Failed force holding (normal), expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that an assign does not change the value when forced.
    #1 assign f = y;
    if (f != 0.0) begin
      $display("Failed force holding (assign), expected 0.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that a force will propagate.
    z = 1.0;
    #1;
    if (f != 1.0) begin
      $display("Failed force propagation, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the release holds the previous value.
    #1 release f;
    if (f != 1.0) begin
      $display("Failed release holding, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that a release correctly breaks the variable link.
    #1 z = 0.0;
    if (f != 1.0) begin
      $display("Failed variable unlinking (force), expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that a deassign holds the previous value.
    #1 deassign f;
    if (f != 1.0) begin
      $display("Failed deassign holding, expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that a deassign correctly breaks the variable link.
    #1 y = 0.0;
    if (f != 1.0) begin
      $display("Failed variable unlinking (deassign), expected 1.0, got %f", f);
      pass = 1'b0;
    end

    // Verify that the value can be changed after a release and a deassign.
    #1 f = 2.0;
    if (f != 2.0) begin
      $display("Failed release, expected 2.0, got %f", f);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
