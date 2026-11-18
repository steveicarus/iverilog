// The definition must have the same number of ports as the prototype.

module test;
  class C;
    extern function int f(int a, int b);
  endclass

  function int C::f(int a);
    return a;
  endfunction
endmodule
