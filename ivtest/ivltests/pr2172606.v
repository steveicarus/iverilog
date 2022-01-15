module main;
  reg passed = 1'b1;
  wire out1;
  reg local_out;
  reg mode;

  assign out1 = mode ? 1'bz : local_out;
  pullup(out1);

  initial begin
    mode = 1'b1;
    local_out = 1'bx;
    // The pull up device sets the level.
    #1 if (out1 !== 1'b1) begin
      $display("FAILED test 1, expected 1'b1, got %b", out1);
      passed = 1'b0;
    end

    mode = 1'b0;
    local_out = 1'b0;
    // Set by local out.
    #1 if (out1 !== local_out) begin
      $display("FAILED test 1, expected %b, got %b", local_out, out1);
      passed = 1'b0;
    end

    local_out = 1'b1;
    // Set by local out.
    #1 if (out1 !== local_out) begin
      $display("FAILED test 1, expected %b, got %b", local_out, out1);
      passed = 1'b0;
    end

    local_out = 1'bx;
    // Set by local out.
    #1 if (out1 !== local_out) begin
      $display("FAILED test 1, expected %b, got %b", local_out, out1);
      passed = 1'b0;
    end

    local_out = 1'bz;
    // The pull up device sets the level.
    #1 if (out1 !== 1'b1) begin
      $display("FAILED test 1, expected 1'b1, got %b", out1);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
