// The definition must use the class handle types of the prototype.

module test;
  class A;
  endclass

  class B;
  endclass

  class C;
    extern function void f(A a);
  endclass

  function void C::f(B a);
  endfunction
endmodule
