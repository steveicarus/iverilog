// Check that %m in an override of a virtual function reports the scope of
// the override, and that an override sees its own arguments and locals.

module test;

  class Base;
    virtual function string name();
      return $sformatf("%m");
    endfunction
    virtual function int calc(int a);
      int t = a;
      return t + 1;
    endfunction
  endclass

  class Derived extends Base;
    function string name();
      return $sformatf("%m");
    endfunction
    function int calc(int a);
      int t = a * 2;
      return t;
    endfunction
  endclass

  Base b;
  Derived d;

  initial begin
    d = new;
    b = d;
    if (b.name() == "test.Derived.name" && b.calc(5) == 10)
      $display("PASSED");
    else
      $display("FAILED: %0s %0d", b.name(), b.calc(5));
  end

endmodule
