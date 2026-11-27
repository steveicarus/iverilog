// Check sizing and sign extension of integral default arguments.

module test;

  bit failed = 1'b0;
  integer task_value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer signed_value(input integer value = -8'sd1);
    return value;
  endfunction

  function integer unsigned_value(input integer value = 8'hff);
    return value;
  endfunction

  function integer sum(input integer value = 8'hff + 8'h01);
    return value;
  endfunction

  function logic [3:0] narrow(input logic [3:0] value = 8'haf);
    return value;
  endfunction

  task save(input integer value = -8'sd1);
    task_value = value;
  endtask

  localparam integer DEFAULT_VALUE = signed_value();

  initial begin
    `check(signed_value(), -1);
    `check(signed_value(-8'sd1), -1);
    `check(DEFAULT_VALUE, -1);
    `check(unsigned_value(), 255);
    `check(sum(), 256);
    `check(sum(8'hff + 8'h01), 256);
    `check(narrow(), 4'hf);
    save();
    `check(task_value, -1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
