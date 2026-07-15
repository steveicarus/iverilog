// Check that a class property can have the same name as the class.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    int C;

    function int get;
      return C;
    endfunction
  endclass

  C obj;

  initial begin
    obj = new;

    obj.C = 23;

    `check(obj.get(), 23);
    `check(obj.C, 23);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
