// Check that a named constructor argument selector can match a visible typedef name.

module test;

  class C;
    integer value;

    function new(input integer T, input integer B);
      value = T + B;
    endfunction
  endclass

  typedef int T;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    C c;

    failed = 1'b0;
    c = new(.T(40), .B(2));
    `check(c.value, 42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
