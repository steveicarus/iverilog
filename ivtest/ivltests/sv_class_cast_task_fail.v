// Check that a failing $cast() used as a task reports a run-time error
// (IEEE 1800-2017 6.24.3) and leaves the destination unchanged.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
  endclass

  Base b;
  Child c;

  initial begin
    b = new();
    c = null;
    $cast(c, b);
    if (c != null)
      $display("FAILED: failed task cast changed the destination");
  end

endmodule
