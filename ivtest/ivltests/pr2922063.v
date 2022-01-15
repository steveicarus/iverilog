module top;
  reg pass;
  integer val;

  initial begin
    pass = 1'b1;

    // Check ARS in a fully signed context.
    // All operands are signed.
    val = -1;
    val = 7'sd10 + (val >>> 1);
    if (val !== 9) begin
      $display("Failed ARS in signed context, got %d", val);
      pass = 1'b0;
    end

    // Check ARS in a cast signed context.
    // This is fully signed as well because of the cast.
    val = -1;
    val = $signed(7'd10) + (val >>> 1);
    if (val !== 9) begin
      $display("Failed ARS in cast signed context, got %d", val);
      pass = 1'b0;
    end
    // Check ARS in a self determined context.
    // The system function is a primary and should create a self-determined
    // context for the ARS. The addition is then done in an unsigned
    // context, but this should still give the correct result.
    //
    // The bug is that Icarus is not sign padding the ARS since the
    // addition is casting it to be unsigned. It should only be able to
    // cast the sign of the result not the actual ARS! This casting is
    // happening in suppress_binary_operand_sign_if_needed() defined in
    // elab_expr.cc. It looks like $signed and $unsigned need some way
    // to protect their argument self-determined context.
    val = -1;
    val = 7'd10 + $signed(val >>> 1);
    if (val !== 9) begin
      $display("Failed ARS in $signed context, got %d", val);
      pass = 1'b0;
    end
    // Check ARS in a different self determined context.
    // See comments above for $signed.
    val = -1;
    val = 7'd10 + $unsigned(val >>> 1);
    if (val !== 9) begin
      $display("Failed ARS in $unsigned context, got %d", val);
      pass = 1'b0;
    end

    // Check ARS in a different self determined context.
    val = -1;
    val = 7'd10 + {val >>> 1};
    if (val !== 9) begin
      $display("Failed ARS in a concatenation context, got %d", val);
      pass = 1'b0;
    end
    if (pass) $display("PASSED");
  end
endmodule
