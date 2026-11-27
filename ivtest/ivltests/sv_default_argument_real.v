// Check that real default arguments are rounded for integral inputs.

module test;

  bit failed = 1'b0;
  integer task_value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer positive(input integer value = 1.5);
    return value;
  endfunction

  function integer negative(input integer value = -1.5);
    return value;
  endfunction

  task save(input integer value = 1.5);
    task_value = value;
  endtask

  initial begin
    `check(positive(), 2);
    `check(positive(1.5), 2);
    `check(negative(), -2);
    `check(negative(-1.5), -2);
    save();
    `check(task_value, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
