// Check that a forward declared class can be used as a base class.

typedef class Middle;
typedef class Base;

class Derived extends Middle;
endclass

class Middle extends Base(43);
endclass

class Base;
  integer constructor_value;

  function new(integer value);
    constructor_value = value;
  endfunction

  function integer value;
    value = 42;
  endfunction
endclass

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", \
               `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end

  Derived object;

  initial begin
    object = new;

    `check(object.value(), 42);
    `check(object.constructor_value, 43);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
