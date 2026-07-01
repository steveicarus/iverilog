// Check that enum literals declared inside functions are available in the
// function scope.

module test;

  reg failed;

  `define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  function logic [3:0] check_func_enum;
    typedef enum logic [3:0] {
      A = 4'd1,
      B = 4'd2
    } EN_T;

    static EN_T e = A;

    e = B;
    check_func_enum = e;
  endfunction

  initial begin
    failed = 1'b0;

    #1;
    `check(check_func_enum(), 4'd2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
