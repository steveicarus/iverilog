// Check two-state conversion of compound assignment results in a packed struct.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef struct packed {
    logic [3:0] four;
    bit [3:0] two;
  } value_t;

  value_t value;

  function value_t convert(input value_t initial_value);
    convert = initial_value;
    convert.two /= 0;
  endfunction

  initial begin
    value = 8'hf5;
    `check(convert(value), 8'hf0);
    value.two /= 0;
    `check(value, 8'hf0);

    value = 8'hf5;
    value.four /= 0;
    `check(value, 8'bxxxx0101);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
