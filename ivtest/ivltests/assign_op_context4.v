// Check that constant functions convert compound assignment results to two-state.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function bit [3:0] add_unknown(input bit [3:0] value);
    value += 4'b000x;
    add_unknown = value;
  endfunction

  function bit [7:0] add_unknown_part(input bit [7:0] value);
    value[7:4] += 4'b000x;
    add_unknown_part = value;
  endfunction

  localparam [7:0] folded_part = add_unknown_part(8'ha5);
  localparam [3:0] folded = add_unknown(5);

  initial begin
    `check(folded, 4'd0);
    `check(folded_part, 8'h05);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
