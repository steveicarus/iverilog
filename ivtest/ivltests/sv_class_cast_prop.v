// Check $cast() with a class property as the destination, both through
// this and through another object.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
  endclass

  class Holder;
    Child c;
    Base b;

    function int load(Base src);
      return $cast(this.c, src);
    endfunction

    function int load_implicit(Base src);
      return $cast(c, src);
    endfunction
  endclass

  Holder h, h2;
  Base b;
  Child c, d;
  int rc;
  bit failed = 0;

  task check(input bit cond, input string msg);
    if (!cond) begin
      $display("FAILED: %s", msg);
      failed = 1;
    end
  endtask

  initial begin
    h = new();
    h2 = new();
    c = new();
    c.y = 5;
    b = c;

    rc = h.load(b);
    check(rc == 1 && h.c != null, "this.c");
    d = h.c;
    check(d.y == 5, "this.c value");

    h.c = null;
    rc = h.load_implicit(b);
    check(rc == 1 && h.c != null, "implicit property");

    rc = $cast(h2.c, b);
    check(rc == 1 && h2.c != null, "obj.c");
    d = h2.c;
    check(d.y == 5, "obj.c value");

    // A failed cast leaves the property alone.
    b = new();
    rc = h.load(b);
    check(rc == 0 && h.c != null, "failed cast this.c");
    d = h.c;
    check(d.y == 5, "failed cast keeps this.c");

    h2.c = null;
    rc = $cast(h2.c, b);
    check(rc == 0 && h2.c == null, "failed cast obj.c");

    // A property can also be the source.
    h.b = c;
    d = null;
    rc = $cast(d, h.b);
    check(rc == 1 && d != null && d.y == 5, "property source");

    if (!failed) $display("PASSED");
  end

endmodule
