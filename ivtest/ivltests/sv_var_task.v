// Check that the var keyword is supported for task ports

bit failed = 1'b0;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", val, exp); \
    failed = 1'b1; \
  end

module test;

  task t1 (var int x);
    `check(x, 10)
  endtask

  task t2 (input var int x, output var int y);
    `check(x, 20)
    y = x;
  endtask

  task t3 (var [7:0] x);
    `check(x, 30)
  endtask

  task t4 (input var [7:0] x, output var [7:0] y);
    `check(x, 40)
    y = x;
  endtask

  initial begin
    int o1;
    logic [7:0] o2;

    t1(10);
    t2(20, o1);
    t3(30);
    t4(40, o2);

    `check(o1, 20)
    `check(o2, 40)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
