// Check enum literals and explicitly cast values as default arguments.

module test;

  typedef enum logic [3:0] {A = 1, B = 2} value_t;
  typedef enum logic [3:0] {OTHER = 2} other_t;
  value_t saved;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function value_t identity(input value_t value = A);
    return value;
  endfunction

  function value_t cast_value(input value_t value = value_t'(4'd2));
    return value;
  endfunction

  function value_t cast_other(input value_t value = value_t'(OTHER));
    return value;
  endfunction

  task save(input value_t value = B);
    saved = value;
  endtask

  initial begin
    `check(identity(), A);
    `check(identity(B), B);
    `check(cast_value(), B);
    `check(cast_other(), B);
    save();
    `check(saved, B);
    save(A);
    `check(saved, A);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
