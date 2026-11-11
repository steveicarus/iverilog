// Check a virtual function when the derived class is declared before
// the base class by using a forward typedef.

module test;

  typedef class Base;

  class Derived extends Base;
    function int val();
      return 2;
    endfunction
  endclass

  class Base;
    virtual function int val();
      return 1;
    endfunction
  endclass

  Base b;
  Derived d;

  initial begin
    b = new;
    d = new;
    if (b.val() != 1) $display("FAILED: base");
    b = d;
    if (b.val() == 2)
      $display("PASSED");
    else
      $display("FAILED: derived");
  end

endmodule
