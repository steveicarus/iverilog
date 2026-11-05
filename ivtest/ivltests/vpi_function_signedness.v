// Check that VPI reports the declared signedness of packed functions.

module test;

  bit failed = 1'b0;

  `define check(name, exp) \
    if ($function_is_signed(name) !== exp) begin \
      $display("FAILED(%0d). '%s' expected signedness %b, got %b", \
               `__LINE__, `"name`", exp, $function_is_signed(name)); \
      failed = 1'b1; \
    end

  function signed [7:0] signed_logic(input dummy);
    signed_logic = 0;
  endfunction

  function [7:0] unsigned_logic(input dummy);
    unsigned_logic = 0;
  endfunction

  function automatic signed [7:0] auto_signed_logic(input dummy);
    auto_signed_logic = 0;
  endfunction

  function automatic [7:0] auto_unsigned_logic(input dummy);
    auto_unsigned_logic = 0;
  endfunction

  function bit signed [7:0] signed_bit(input dummy);
    signed_bit = 0;
  endfunction

  function bit [7:0] unsigned_bit(input dummy);
    unsigned_bit = 0;
  endfunction

  function automatic bit signed [7:0] auto_signed_bit(input dummy);
    auto_signed_bit = 0;
  endfunction

  function automatic bit [7:0] auto_unsigned_bit(input dummy);
    auto_unsigned_bit = 0;
  endfunction

  initial begin
    `check(test.signed_logic, 1'b1);
    `check(test.unsigned_logic, 1'b0);
    `check(test.auto_signed_logic, 1'b1);
    `check(test.auto_unsigned_logic, 1'b0);
    `check(test.signed_bit, 1'b1);
    `check(test.unsigned_bit, 1'b0);
    `check(test.auto_signed_bit, 1'b1);
    `check(test.auto_unsigned_bit, 1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
