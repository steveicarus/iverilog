// Check assignment patterns in default arguments.

module test;

  typedef struct packed {logic [3:0] a, b;} pair_t;
  pair_t task_value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function pair_t named(input pair_t value = '{a: 3, b: 5});
    return value;
  endfunction

  function pair_t positional(input pair_t value = '{3, 5});
    return value;
  endfunction

  function pair_t filled(input pair_t value = '{default: '1});
    return value;
  endfunction

  task save(input pair_t value = '{a: 3, b: 5});
    task_value = value;
  endtask

  initial begin
    `check(named(), 8'h35);
    `check(named('{a: 3, b: 5}), 8'h35);
    `check(positional(), 8'h35);
    `check(positional('{3, 5}), 8'h35);
    `check(filled(), 8'hff);
    `check(filled('{default: '1}), 8'hff);
    save();
    `check(task_value, 8'h35);
    save('{a: 3, b: 5});
    `check(task_value, 8'h35);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
