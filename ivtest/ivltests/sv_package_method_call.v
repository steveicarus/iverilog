// Check method calls on package-scoped objects and queues.

package p;
  class C;
    integer value = 0;

    task set(input integer new_value);
      value = new_value;
    endtask

    function void add(input integer increment);
      value += increment;
    endfunction

    function integer set_function(input integer new_value);
      value = new_value;
      set_function = value;
    endfunction
  endclass

  C c = new;
  integer values[$];
endpackage

module test;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    p::c.set(10);
    p::c.add(20);
    void'(p::c.set_function(42));
    `check(p::c.value, 42);

    p::values.push_back(17);
    p::values.push_back(25);
    void'(p::values.pop_front());
    `check(p::values.size(), 1);
    `check(p::values[0], 25);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
