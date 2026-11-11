// Check that overrides of virtual methods with a matching signature are
// accepted, including a covariant class return type, and that calls
// through a handle of the derived type reach the override.

module test;

  class A;
    int id = 1;
  endclass

  class B extends A;
    function new();
      id = 2;
    endfunction
  endclass

  class Base;
    virtual function A make(int a, real r, string s);
      A res = new;
      return res;
    endfunction
    virtual task set(input int a, output int b);
      b = a;
    endtask
  endclass

  class Derived extends Base;
    function B make(int a, real r, string s);
      B res = new;
      return res;
    endfunction
    task set(input int a, output int b);
      b = a + 1;
    endtask
  endclass

  Derived d;
  B b;
  int v;

  initial begin
    d = new;
    b = d.make(1, 2.0, "x");
    d.set(4, v);
    if (b.id == 2 && v == 5)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
