module top;
  // We expect <integer_width> bits for the zero or mone parameters. The
  // big test must also be at least <integer_width>, but Icarus currently
  // creates 48 bits. I personally think this is the correct behavior.
  parameter zero = 0;
  parameter mone = -1;
  parameter big = 'hffffffffffff;

  parameter max = 2**16; // We will call this many bits unlimited.
  reg pass;
  integer idx;

  initial begin
    pass = 1'b1;

    /*
     * Check with a bit select.
     */
    $display("Checking the size with a bit select.");
    $display("------------------------------------");
    // Test to see how far a decimal 0 is extended.
    begin: loop_zero
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (zero[idx] !== 0) disable loop_zero;
      end
    end
    if (idx != max) begin
      $display("The size of a decimal 0 parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (zero[idx] !== 1'bx) begin
        $display("  Failed: after bit must be 1'bx, got %b.", zero[idx]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a decimal 0 parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (zero[idx] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", zero[idx]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (zero[idx] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", zero[idx]);
      pass = 1'b0;
    end

    // Test to see how far a decimal -1 is extended.
    begin: loop_mone
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (mone[idx] !== 1) disable loop_mone;
      end
    end
    if (idx != max) begin
      $display("The size of a decimal -1 parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (mone[idx] !== 1'bx) begin
        $display("  Failed: after bit must be 1'bx, got %b", mone[idx]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a decimal -1 parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (mone[idx] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", mone[idx]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (mone[idx] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", mone[idx]);
      pass = 1'b0;
    end

    // Check to see if a parameter can be more than 32 bits (I expect
    // unlimited or 48 bits). If they exist the first 48 bits must be 1
    // any remaining bits are 0.
    begin: loop_big
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (big[idx] !== (idx < 48)) disable loop_big;
      end
    end
    if (idx != max) begin
      $display("The size of a big decimal parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (big[idx] !== 1'bx) begin
        $display("  Failed: after bit must be 1'bx, got %b", big[idx]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a big decimal parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 48) begin
      $display("  Warning: 48 bit unsized parameter was truncated to %0d bits",
               idx);
    end
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (big[idx] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", big[idx]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (big[idx] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", big[idx]);
      pass = 1'b0;
    end


    /*
     * Check with an indexed up select.
     */
    $display("");
    $display("Checking the size with an indexed part select.");
    $display("----------------------------------------------");
    // Test to see how far a decimal 0 is extended.
    begin: loop_zero2
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (zero[idx+:1] !== 0) disable loop_zero2;
      end
    end
    if (idx != max) begin
      $display("The size of a decimal 0 parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (zero[idx+:1] !== 1'bx) begin
        $display("  Failed: after bit must be 1'bx, got %b", zero[idx+:1]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a decimal 0 parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (zero[idx+:1] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", zero[idx+:1]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (zero[idx+:1] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", zero[idx+:1]);
      pass = 1'b0;
    end

    // Test to see how far a decimal -1 is extended.
    begin: loop_mone2
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (mone[idx+:1] !== 1) disable loop_mone2;
      end
    end
    if (idx != max) begin
      $display("The size of a decimal -1 parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (mone[idx+:1] !== 1'bx) begin
        $display("  Failed: after bit must be 1'bx, got %b", mone[idx+:1]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a decimal -1 parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (mone[idx+:1] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", mone[idx+:1]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (mone[idx+:1] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", mone[idx+:1]);
      pass = 1'b0;
    end

    // Check to see if a parameter can be more than 32 bits (I expect
    // unlimited or 48 bits). If they exist the first 48 bits must be 1
    // any remaining bits are 0.
    begin: loop_big2
      for (idx = 0; idx < max; idx = idx + 1) begin
        if (big[idx+:1] !== (idx < 48)) disable loop_big2;
      end
    end
    if (idx != max) begin
      $display("The size of a big decimal parameter is %0d bits.", idx);
      // Check that after the parameter is 1'bx.
      if (big[idx+:1] !== 1'bx) begin
        $display("Failed: after bit must be 1'bx, got %b", big[idx+:1]);
        pass = 1'b0;
      end
    end else begin
      $display("The size of a big decimal parameter is unlimited.");
    end
    // An unsized parameter must be at least 32 bits.
    if (idx < 48) begin
      $display("  Warning: 48 bit unsized parameter was truncated to %0d bits",
               idx);
    end
    if (idx < 32) begin
      $display("  Failed: unsized parameter must be >= 32 bits, got %0d.", idx);
      pass = 1'b0;
    end
    // Check that before the parameter is 1'bx.
    idx = -1;
    if (big[idx+:1] !== 1'bx) begin
      $display("  Failed: before bit must be 1'bx, got %b.", big[idx+:1]);
      pass = 1'b0;
    end
    // Check that an undefined index gives 1'bx.
    idx = 'bx;
    if (big[idx+:1] !== 1'bx) begin
      $display("  Failed: undefined select must be 1'bx, got %b.", big[idx+:1]);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
