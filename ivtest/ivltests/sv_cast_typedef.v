// Check that type cast works as expected when using type identifiers.

module test;

  typedef string T_S;
  typedef real T_R;
  typedef logic [15:0] T_V;

  string s;
  real r;
  logic [15:0] v;

  bit failed;

  `define check(expr, val) \
    if (expr != val) begin \
      $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    v = "Hi";
    s = T_S'(v);

    `check(s, "Hi")

    v = 123;
    r = T_R'(v);

    `check(r, 123)

    r = 1.23;
    v = T_V'(r);

    `check(v, 16'd1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
