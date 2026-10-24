// Check that strength-bearing nets drive both RCMOS control inputs.

module test;
  reg n, p;
  wire n_weak, p_weak, result;
  reg failed;
  assign (weak0, weak1) n_weak = n;
  assign (weak0, weak1) p_weak = p;
  rcmos gate(result, 1'b1, n_weak, p_weak);
  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    n = 0; p = 1;
    #1; `check(result, 1'bz);
    p = 0;
    #1; `check(result, 1'b1);
    p = 1;
    #1; `check(result, 1'bz);
    n = 1;
    #1; `check(result, 1'b1);
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
