// Check that $cast() with a destination that is not a class variable
// is a compile time error.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
  endclass

  Base b;
  Child c;
  int rc;

  function Child get();
    return c;
  endfunction

  initial begin
    b = new();
    rc = $cast(get(), b);
    $display("FAILED");
  end

endmodule
