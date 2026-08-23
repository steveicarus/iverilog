// Check relaxed declaration ordering for ambiguous and required type names.
// This is not valid strict SystemVerilog and tests
// -gno-strict-net-var-declaration.

typedef logic [7:0] ambiguous_name;
typedef logic [15:0] required_type;

module test;

  required_type required_value;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check($bits(ambiguous_name), 32);
    `check($bits(required_value), 16);

    if (!failed) begin
      $display("PASSED");
    end
  end

  integer ambiguous_name;
  integer required_type;

endmodule
