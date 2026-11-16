// Check that compound assignments convert only two-state concatenation components.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  struct packed {
    logic [3:0] four;
    bit [3:0] two;
  } value;

  initial begin
    value = 8'hf5;
    {value.four, value.two} &= 8'bxxxxxxxx;
    `check(value, 8'bxxxx0000);
    value = 8'hf5;
    {value.two, value.four} &= 8'bxxxxxxxx;
    `check(value, 8'bxxxx0000);
    value = 8'h05;
    {value.two, value.four} |= 8'bzzzzzzzz;
    `check(value, 8'bxxxx0101);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
