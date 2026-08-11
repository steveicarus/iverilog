// Check lexical ordering when an identifier can be a type or a value.

integer value;
typedef logic [7:0] type_name;

module test;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check($bits(value), 32);
    `check($bits(type_name), 8);

    if (!failed) begin
      $display("PASSED");
    end
  end

  typedef logic [3:0] value;
  integer type_name;

endmodule
