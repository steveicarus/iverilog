// Check that a named task argument selector can match a visible typedef name.

module test;

  task t(input integer T, output integer result);
    result = T;
  endtask

  typedef int T;

  reg failed;
  integer value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    t(.T(42), .result(value));
    `check(value, 42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
