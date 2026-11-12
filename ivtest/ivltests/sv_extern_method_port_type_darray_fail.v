// The element type of a dynamic array port must match the prototype.

module test;
  class C;
    extern function void f(int a[]);
  endclass

  function void C::f(byte a[]);
  endfunction
endmodule
