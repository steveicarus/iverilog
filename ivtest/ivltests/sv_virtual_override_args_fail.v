// Check that an override of a virtual method must have the same number
// of arguments. The override does not repeat the virtual keyword, it is
// virtual because the method in the base class is.

module test;

  class Base;
    virtual function int f(int a);
      return a;
    endfunction
  endclass

  class Derived extends Base;
    function int f(int a, int b);
      return a + b;
    endfunction
  endclass

endmodule
