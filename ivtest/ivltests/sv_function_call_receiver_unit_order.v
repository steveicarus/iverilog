// Check lexical ordering for the receiver of a function call.

class C;

  function integer value;
    value = 11;
  endfunction

endclass

C before_object = new;

module holder;

  function integer value;
    value = 22;
  endfunction

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

    before_value = before_object.value();
    after_value = after_object.value();

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
