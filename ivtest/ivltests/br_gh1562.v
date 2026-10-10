// Check that logical implication short circuits a false left operand.

module test;

  reg left, right, result;
  integer left_calls, right_calls;
  bit failed = 1'b0;

  function lhs(input value);
    begin
      left_calls = left_calls + 1;
      lhs = value;
    end
  endfunction

  function rhs(input value);
    begin
      right_calls = right_calls + 1;
      rhs = value;
    end
  endfunction

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_implies(l, r, exp, calls) \
    left = l; \
    right = r; \
    left_calls = 0; \
    right_calls = 0; \
    result = (lhs(left) -> rhs(right)); \
    `check(result, exp); \
    `check(left_calls, 1); \
    `check(right_calls, calls);

  initial begin
    `check_implies(1'b0, 1'b0, 1'b1, 0);
    `check_implies(1'b0, 1'bx, 1'b1, 0);
    `check_implies(1'b1, 1'b0, 1'b0, 1);
    `check_implies(1'b1, 1'b1, 1'b1, 1);
    `check_implies(1'bx, 1'b0, 1'bx, 1);
    `check_implies(1'bx, 1'b1, 1'b1, 1);
    `check_implies(1'bz, 1'b0, 1'bx, 1);
    `check_implies(1'bz, 1'b1, 1'b1, 1);

    left = 1'b0;
    left_calls = 0;
    right_calls = 0;
    result = 1'b0;
    if (lhs(left) -> rhs(right)) result = 1'b1;
    `check(result, 1'b1);
    `check(left_calls, 1);
    `check(right_calls, 0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
