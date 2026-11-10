// Check $cast() through several levels of inheritance and between
// sibling classes.

module test;

  class A;
    int a;
  endclass

  class B extends A;
    int b;
  endclass

  class C extends B;
    int c;
  endclass

  class D extends A;
    int d;
  endclass

  A pa;
  B pb;
  C pc;
  D pd;
  int rc;
  bit failed = 0;

  task check(input bit cond, input string msg);
    if (!cond) begin
      $display("FAILED: %s", msg);
      failed = 1;
    end
  endtask

  initial begin
    pc = new();
    pc.a = 1;
    pc.b = 2;
    pc.c = 3;

    // A C object seen through an A handle casts to B and C.
    pa = pc;
    pb = null;
    rc = $cast(pb, pa);
    check(rc == 1 && pb != null && pb.b == 2, "A to B holding a C");
    pc = null;
    rc = $cast(pc, pa);
    check(rc == 1 && pc != null && pc.c == 3 && pc.b == 2 && pc.a == 1,
          "A to C holding a C");

    // The B view of the same object casts to C as well.
    pc = null;
    rc = $cast(pc, pb);
    check(rc == 1 && pc != null && pc.c == 3, "B to C holding a C");

    // A B object does not cast to C.
    pb = new();
    pa = pb;
    pc = null;
    rc = $cast(pc, pa);
    check(rc == 0 && pc == null, "A to C holding a B");
    rc = $cast(pc, pb);
    check(rc == 0 && pc == null, "B to C holding a B");

    // Siblings fail through the common base handle.
    pd = new();
    pa = pd;
    pb = null;
    rc = $cast(pb, pa);
    check(rc == 0 && pb == null, "A to B holding a D");

    pc = new();
    pa = pc;
    pd = null;
    rc = $cast(pd, pa);
    check(rc == 0 && pd == null, "A to D holding a C");

    pa = new();
    rc = $cast(pd, pa);
    check(rc == 0 && pd == null, "A to D holding an A");

    if (!failed) $display("PASSED");
  end

endmodule
