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

  int d_new_calls = 0;

  class C;
    int x = 10;
  endclass

  class D extends C;
    function new;
      d_new_calls++;
    endfunction
  endclass

  class E extends D;
    int y;
    function new;
      y = 20;
    endfunction
  endclass

  initial begin
    E e;
    e = new;

    `check(d_new_calls, 1);
    `check(e.x, 10);
    `check(e.y, 20);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
