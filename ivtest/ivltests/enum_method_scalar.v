// Check that enum methods work on scalar enums.

module test;
  enum bit signed {NEG = -1, ZERO = 0} value;
  enum logic {L_ZERO = 0, L_ONE = 1, L_X = 1'bx, L_Z = 1'bz} logic_value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_name(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_methods(value, member, next_member, prev_member) \
    value = member; \
    `check_name(value.name(), `"member`"); \
    `check(value.next(), next_member); \
    `check(value.prev(), prev_member); \
    `check(value.next(0), member); \
    `check(value.prev(0), member);

  initial begin
    `check_methods(value, NEG, ZERO, ZERO);
    `check_methods(value, ZERO, NEG, NEG);
    `check_methods(logic_value, L_ZERO, L_ONE, L_Z);
    `check_methods(logic_value, L_ONE, L_X, L_ZERO);
    `check_methods(logic_value, L_X, L_Z, L_ONE);
    `check_methods(logic_value, L_Z, L_ZERO, L_X);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
