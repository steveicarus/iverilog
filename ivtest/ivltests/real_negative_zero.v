// Check that the vvp code generator preserves the sign of a negative zero
// real constant. The sign used to be detected with (value < 0), which is
// false for IEEE 754 -0.0, so a -0.0 constant was emitted as +0.0 and the
// compiled value no longer matched the runtime real value.

module test;

  real nz;
  real pz;

  initial begin
    nz = -0.0;
    pz = 0.0;

    // The sign bit is the only thing that tells -0.0 from +0.0.
    if ($realtobits(nz) !== 64'h8000_0000_0000_0000) begin
      $display("FAILED: -0.0 stored as %h", $realtobits(nz));
      $finish;
    end

    if ($realtobits(pz) !== 64'h0000_0000_0000_0000) begin
      $display("FAILED: 0.0 stored as %h", $realtobits(pz));
      $finish;
    end

    // The reciprocal makes the sign observable: 1.0/-0.0 is -inf.
    if (1.0 / nz >= 0.0) begin
      $display("FAILED: 1.0/-0.0 = %g", 1.0 / nz);
      $finish;
    end

    $display("PASSED");
  end

endmodule
