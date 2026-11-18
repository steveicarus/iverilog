// Check $cast() of objects that are reached through virtual dispatch:
// the cast looks at the class of the object, not at the type of the
// handle.

module test;

  class Shape;
    virtual function string name();
      return "shape";
    endfunction
    virtual function Shape self();
      return this;
    endfunction
  endclass

  class Circle extends Shape;
    function string name();
      return "circle";
    endfunction
  endclass

  class Square extends Shape;
    function string name();
      return "square";
    endfunction
  endclass

  Shape s;
  Circle ci;
  Square sq;
  Shape list[2];
  int rc;
  bit failed = 0;

  task check(input bit cond, input string msg);
    if (!cond) begin
      $display("FAILED: %s", msg);
      failed = 1;
    end
  endtask

  initial begin
    ci = new();
    sq = new();
    list[0] = ci;
    list[1] = sq;

    // The handle returned by a virtual function is a Shape handle.
    s = ci.self();
    sq = null;
    rc = $cast(sq, s);
    check(rc == 0 && sq == null, "Circle is not a Square");
    ci = null;
    rc = $cast(ci, s);
    check(rc == 1 && ci != null && ci.name() == "circle", "Circle is a Circle");

    s = list[1];
    ci = null;
    rc = $cast(ci, s);
    check(rc == 0 && ci == null, "list[1] is not a Circle");
    rc = $cast(sq, s);
    check(rc == 1 && sq != null && sq.name() == "square", "list[1] is a Square");

    // Same, using the result of a function call as the source.
    s = list[0];
    sq = null;
    rc = $cast(sq, s.self());
    check(rc == 0 && sq == null, "self() of a Circle is not a Square");
    rc = $cast(ci, s.self());
    check(rc == 1 && ci != null && ci.name() == "circle",
          "self() of a Circle is a Circle");

    if (!failed) $display("PASSED");
  end

endmodule
