// Check $cast() used as a task. A successful cast assigns the
// destination. A failed cast is a run-time error and leaves the
// destination unchanged.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
  endclass

  Base b;
  Child c, c2;

  initial begin
    c = new();
    c.y = 7;
    b = c;
    c2 = null;
    $cast(c2, b);
    if (c2 == null || c2.y != 7) begin
      $display("FAILED: task cast did not assign");
      $finish;
    end

    b = new();
    c2 = c;
    $cast(c2, b);
    if (c2 == null || c2.y != 7) begin
      $display("FAILED: failed task cast changed the destination");
      $finish;
    end

    $display("PASSED");
  end

endmodule
