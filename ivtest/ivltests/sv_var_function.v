// Check that the var keyword is supported for function ports

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", val, exp); \
    return 1'b1; \
  end \
  return 1'b0;

module test;

  function bit f1 (var int x);
    `check(x, 10)
  endfunction

  function bit f2 (input var int x);
    `check(x, 20)
  endfunction

  function bit f3 (var [7:0] x);
    `check(x, 30)
  endfunction

  function bit f4 (input var [7:0] x);
    `check(x, 40)
  endfunction

  initial begin
    bit failed;

    failed = f1(10);
    failed |= f2(20);
    failed |= f3(30);
    failed |= f4(40);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
