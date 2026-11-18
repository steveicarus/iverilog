// Check that $cast() with "this" as the destination is a compile time
// error, as "this" cannot be assigned.

module test;

  class Base;
    int x;
  endclass

  class Child extends Base;
    int y;
    function void f(Base b);
      $cast(this, b);
    endfunction
  endclass

  initial begin
    $display("FAILED");
  end

endmodule
