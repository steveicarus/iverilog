// Check legal $unit:: references and a forward function reference.

parameter integer parameter_value = 17;
integer variable_value = 42;
event event_value;

module test;

  reg event_seen;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  always @($unit::event_value)
    event_seen = 1'b1;

  initial begin
    failed = 1'b0;
    event_seen = 1'b0;

    #1;
    -> $unit::event_value;
    #1;

    `check($unit::parameter_value, 17);
    `check($unit::variable_value, 42);
    `check(event_seen, 1'b1);
    `check($unit::function_value(), 23);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule

function integer function_value;
  function_value = 23;
endfunction
