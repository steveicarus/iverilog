// Check that the constructor of a base class gets called exactly once when
// mixing implicit and explicit constructors.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do begin \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): `%s`, expected `%0d`, got `%0d`.", `__LINE__, \
               `"val`", (exp), (val)); \
      failed = 1'b1; \
    end \
  end while (0)

  int c_new_calls = 0;

  class C;
    function new;
      c_new_calls++;
    endfunction
  endclass

  class D extends C;
    int x = 10;
  endclass

  initial begin
    D d;
    d = new;

    `check(c_new_calls, 1);
    `check(d.x, 10);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
