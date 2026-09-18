// Check that a compilation-unit function takes precedence over a function in
// an enclosing instance.

module child;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check($bits(value()), 4)
    `check(value(), 4'h5)
    `check($bits(instance_only()), 3)
    `check(instance_only(), 3'h6)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule

function [3:0] value;
  value = 4'h5;
endfunction

module test;

  child i_child();

  function [7:0] value;
    value = 8'ha5;
  endfunction

  function [2:0] instance_only;
    instance_only = 3'h6;
  endfunction

endmodule
