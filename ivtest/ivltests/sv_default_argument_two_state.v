// Check that X and Z default argument bits convert to zero for two-state inputs.

module test;

  logic [3:0] task_value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function logic [3:0] bit_value(input bit [3:0] value = 4'bxz01);
    return value;
  endfunction

  function integer int_value(input int value = 32'hxz000001);
    return value;
  endfunction

  function logic [64:0] wide_value(
    input bit [64:0] value = {1'bx, 32'hffffffff, 32'bz});
    return value;
  endfunction

  task save(input bit [3:0] value = 4'bxz01);
    task_value = value;
  endtask

  initial begin
    `check(bit_value(), 4'b0001);
    `check(bit_value(4'bxz01), 4'b0001);
    `check(int_value(), 1);
    `check(int_value(32'hxz000001), 1);
    `check(wide_value(), 65'h0ffffffff00000000);
    `check(wide_value({1'bx, 32'hffffffff, 32'bz}), 65'h0ffffffff00000000);
    save();
    `check(task_value, 4'b0001);
    save(4'bxz01);
    `check(task_value, 4'b0001);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
