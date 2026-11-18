// Check that $cast() of a null handle succeeds and makes the
// destination null.

module test;

  class Base;
  endclass

  class Child extends Base;
  endclass

  Base b;
  Child c;
  int rc;

  initial begin
    c = new();
    b = null;
    rc = $cast(c, b);
    if (rc !== 1 || c != null) begin
      $display("FAILED: null source, rc=%0d", rc);
      $finish;
    end

    c = new();
    rc = $cast(c, null);
    if (rc !== 1 || c != null) begin
      $display("FAILED: null literal, rc=%0d", rc);
      $finish;
    end

    $display("PASSED");
  end

endmodule
