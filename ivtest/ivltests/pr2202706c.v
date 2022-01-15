module top;
  reg passed;
  reg [9*8:1] check;
  reg [71:0] value;
  reg [7:0] nval;
  real rval;

  initial begin
    passed = 1'b1;
    // Look for the hex value using a runtime string.
    check = "hex=%h";
    if (! $value$plusargs(check, value)) begin
      $display("FAILED: Unable to get hex value.");
      passed = 1'b0;
    end
    if (value !== 72'h0123456789abcdefxz) begin
      $display("FAILED: expected hex value 72'h0123456789abcdefxz, got %h",
               value);
      passed = 1'b0;
    end

    // Look for a hex (x) value.
    if (! $value$plusargs("hex=%x", value)) begin
      $display("FAILED: Unable to get hex value.");
      passed = 1'b0;
    end
    if (value !== 72'h0123456789abcdefxz) begin
      $display("FAILED: expected hex value 72'h0123456789abcdefxz, got %h",
               value);
      passed = 1'b0;
    end

    // Look for an octal value.
    if (! $value$plusargs("oct=%o", value)) begin
      $display("FAILED: Unable to get octal value.");
      passed = 1'b0;
    end
    if (value !== 72'o01234567xz) begin
      $display("FAILED: expected octal value 72'o01234567xz, got %o",
               value);
      passed = 1'b0;
    end

    // Look for a binary value.
    if (! $value$plusargs("bin=%b", value)) begin
      $display("FAILED: Unable to get binary value.");
      passed = 1'b0;
    end
    if (value !== 72'b0101xz) begin
      $display("FAILED: expected binary value 72'b0101xz, got %b",
               value);
      passed = 1'b0;
    end

    // Look for a negative binary value.
    if (! $value$plusargs("neg=%b", value)) begin
      $display("FAILED: Unable to get negative binary value.");
      passed = 1'b0;
    end
    if (value !== 72'hfffffffffffffffffc) begin
      $display("FAILED: expected binary value 72'hff...fc, got %h",
               value);
      passed = 1'b0;
    end

    // Look for a negative octal value.
    if (! $value$plusargs("neg=%o", value)) begin
      $display("FAILED: Unable to get negative octal value.");
      passed = 1'b0;
    end
    if (value !== 72'hffffffffffffffffc0) begin
      $display("FAILED: expected octal value 72'hff...fc0, got %h",
               value);
      passed = 1'b0;
    end

    // Look for a truncated negative hex value.
    if (! $value$plusargs("neg=%h", nval)) begin
      $display("FAILED: Unable to get negative hex value.");
      passed = 1'b0;
    end
    if (nval !== 8'h00) begin
      $display("FAILED: expected hex value 8'h00, got %h",
               nval);
      passed = 1'b0;
    end

    // Look for a bad binary value.
    if (! $value$plusargs("bad_num=%b", value)) begin
      $display("FAILED: Unable to get bad binary value.");
      passed = 1'b0;
    end
    if (value !== 'bx) begin
      $display("FAILED: expected bad binary value 'bx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a bad octal value.
    if (! $value$plusargs("bad_num=%o", value)) begin
      $display("FAILED: Unable to get bad octal value.");
      passed = 1'b0;
    end
    if (value !== 'bx) begin
      $display("FAILED: expected bad octal value 'bx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a bad hex value.
    if (! $value$plusargs("bad_num=%h", value)) begin
      $display("FAILED: Unable to get bad hex value.");
      passed = 1'b0;
    end
    if (value !== 'bx) begin
      $display("FAILED: expected bad hex value 'bx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a bad hex (x) value.
    if (! $value$plusargs("bad_num=%x", value)) begin
      $display("FAILED: Unable to get bad hex (x) value.");
      passed = 1'b0;
    end
    if (value !== 'bx) begin
      $display("FAILED: expected bad hex (x) value 'bx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a decimal value.
    if (! $value$plusargs("dec=%d", value)) begin
      $display("FAILED: Unable to get decimal value.");
      passed = 1'b0;
    end
    if (value !== 'd0123456789) begin
      $display("FAILED: expected decimal value 'd0123456789, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a negative decimal value.
    if (! $value$plusargs("neg=%d", value)) begin
      $display("FAILED: Unable to get negative decimal value.");
      passed = 1'b0;
    end
    if (value !== -100) begin
      $display("FAILED: expected decimal value 72'hff...fc0, got %h",
               value);
      passed = 1'b0;
    end

    // Look for a bad decimal value.
    if (! $value$plusargs("bad_num=%d", value)) begin
      $display("FAILED: Unable to get bad decimal value.");
      passed = 1'b0;
    end
    if (value !== 'bx) begin
      $display("FAILED: expected bad decimal value 'bx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a decimal "x" value.
    if (! $value$plusargs("dec_x=%d", value)) begin
      $display("FAILED: Unable to get decimal \"x\" value.");
      passed = 1'b0;
    end
    if (value !== 'dx) begin
      $display("FAILED: expected decimal value 'dx, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a decimal "z" value.
    if (! $value$plusargs("dec_z=%d", value)) begin
      $display("FAILED: Unable to get decimal \"z\" value.");
      passed = 1'b0;
    end
    if (value !== 'dz) begin
      $display("FAILED: expected decimal value 'dz, got %d",
               value);
      passed = 1'b0;
    end

    // Look for a real value.
    if (! $value$plusargs("real=%f", rval)) begin
      $display("FAILED: Unable to get real value.");
      passed = 1'b0;
    end
    if (rval != 12.3456789) begin
      $display("FAILED: expected real value 12.3456789, got %f",
               rval);
      passed = 1'b0;
    end

    // Look for a negative real value.
    if (! $value$plusargs("neg_real=%f", rval)) begin
      $display("FAILED: Unable to get a negative real value.");
      passed = 1'b0;
    end
    if (rval != -23456.0) begin
      $display("FAILED: expected negative real value -23456.0, got %f",
               rval);
      passed = 1'b0;
    end

    // Look for an infinite real value.
    if (! $value$plusargs("real_inf=%f", rval)) begin
      $display("FAILED: Unable to get infinite real value.");
      passed = 1'b0;
    end
    if (rval != 1.0/0.0) begin
      $display("FAILED: expected infinite real value Inf, got %f",
               rval);
      passed = 1'b0;
    end

    // Look for a bad real value.
    if (! $value$plusargs("bad_num=%f", rval)) begin
      $display("FAILED: Unable to get bad real value.");
      passed = 1'b0;
    end
    if (rval != 0.0) begin
      $display("FAILED: expected bad real value 0.0, got %f",
               rval);
      passed = 1'b0;
    end

    // Look for a warning real value.
    if (! $value$plusargs("warn_real=%f", rval)) begin
      $display("FAILED: Unable to get warning real value.");
      passed = 1'b0;
    end
    if (rval != 9.825) begin
      $display("FAILED: expected warning real value 9.825, got %f",
               rval);
      passed = 1'b0;
    end

    // Put a decimal value into a real based value.
    if (! $value$plusargs("dec=%d", rval)) begin
      $display("FAILED: Unable to get decimal (real) value.");
      passed = 1'b0;
    end
    if (rval != 123456789.0) begin
      $display("FAILED: expected decimal as real value 12...89.0, got %f",
               rval);
      passed = 1'b0;
    end

    // Put a negative decimal into a real based value.
    if (! $value$plusargs("neg=%d", rval)) begin
      $display("FAILED: Unable to get negative decimal (real) value.");
      passed = 1'b0;
    end
    if (rval != -100.0) begin
      $display("FAILED: expected decimal as real value -100, got %f",
               rval);
      passed = 1'b0;
    end

    // Put a real value into a bit based value.
    if (! $value$plusargs("real=%f", value)) begin
      $display("FAILED: Unable to get real (bit) value.");
      passed = 1'b0;
    end
    if (value !== 12) begin
      $display("FAILED: expected real as bit value 12, got %d",
               value);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
