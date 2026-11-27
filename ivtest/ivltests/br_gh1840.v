// Check that '1 default arguments fill the formal argument width.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  task automatic t1(input logic [31:0] value = '1);
    `check(value, 32'hffffffff);
  endtask

  function automatic logic [31:0] f1(input logic [31:0] value = '1);
    return value;
  endfunction

  task automatic t2(input logic [31:0] value = -1);
    `check(value, 32'hffffffff);
  endtask

  initial begin
    t1();
    `check(f1(), 32'hffffffff);
    t1('1);
    t2();

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
