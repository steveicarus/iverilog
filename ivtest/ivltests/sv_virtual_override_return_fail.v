// Check that an override of a virtual method must have the same return
// type. Only class handles may differ, and then only to a derived class.

module test;

  class A;
  endclass

  class B;
  endclass

  class Base;
    virtual function A f();
      return null;
    endfunction
  endclass

  class Derived extends Base;
    virtual function B f();
      return null;
    endfunction
  endclass

endmodule
