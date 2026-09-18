// Check lexical ordering for the receiver of a task call.

class C;

  task set_value(output integer result);
    result = 11;
  endtask

endclass

C before_object = new;

module holder;

  task set_value;
    output integer result;
    result = 22;
  endtask

endmodule

module child;

  integer before_value;
  integer after_value;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    before_object.set_value(before_value);
    after_object.set_value(after_value);

    `check(before_value, 11)
    `check(after_value, 22)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder before_object();
  holder after_object();
  child child_instance();

endmodule

C after_object = new;
