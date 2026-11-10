// Check $cast() of class handles used as a function: the cast succeeds
// when the object is an instance of the destination class or a class
// derived from it, and otherwise returns 0 and leaves the destination
// unchanged.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
  endclass

  Base b, b2;
  Child c, c2, c3;
  int rc;
  bit failed = 0;

  task check(input bit cond, input string msg);
    if (!cond) begin
      $display("FAILED: %s", msg);
      failed = 1;
    end
  endtask

  initial begin
    c = new();
    c.x = 10;
    c.y = 20;

    // Downcast of a handle that holds a Child object.
    b = c;
    c2 = null;
    rc = $cast(c2, b);
    check(rc == 1, "downcast of a Child returns 1");
    check(c2 != null, "downcast assigns the destination");
    check(c2.x == 10 && c2.y == 20, "downcast keeps the properties");

    // The destination is the same object, not a copy.
    c2.y = 21;
    check(c.y == 21, "downcast does not copy the object");

    // Upcast and same class.
    b2 = null;
    rc = $cast(b2, c);
    check(rc == 1 && b2 != null && b2.x == 10, "upcast");
    b2 = null;
    rc = $cast(b2, b);
    check(rc == 1 && b2 != null && b2.x == 10, "same class");

    // Downcast of a handle that holds a plain Base object.
    b = new();
    b.x = 99;
    c3 = c;
    rc = $cast(c3, b);
    check(rc == 0, "downcast of a Base returns 0");
    check(c3 != null && c3.y == 21, "failed cast leaves the destination unchanged");

    c3 = null;
    rc = $cast(c3, b);
    check(rc == 0 && c3 == null, "failed cast leaves a null destination null");

    // The result can be used in a condition directly.
    b = c;
    c2 = null;
    if ($cast(c2, b)) check(c2 != null && c2.y == 21, "cast in a condition");
    else check(0, "cast in a condition should succeed");

    b = new();
    c2 = null;
    if ($cast(c2, b)) check(0, "cast in a condition should fail");
    check(c2 == null, "failed cast in a condition");

    if (!failed) $display("PASSED");
  end

endmodule
