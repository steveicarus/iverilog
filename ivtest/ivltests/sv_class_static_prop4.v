// Check that it is possible to access static properties of a base class

class B;
  static int x = 0;
  static int y = 0;
endclass

class C extends B;

  task t(int a, int b);
    x = a;
    this.y = b;
  endtask

  function int f;
    return x + this.y;
  endfunction

endclass

module test;
  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: `%s`, expected %b, got %b", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  C c = new;

  initial begin
    // Check access inside the class
    c.t(10, 20);
    `check(c.f(), 30)

    // Check access outside of the class
    c.x = 40;
    `check(c.x, 40)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
