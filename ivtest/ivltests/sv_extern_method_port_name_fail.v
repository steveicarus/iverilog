// The definition must use the port names of the prototype.

module test;
  class C;
    extern function int f(int a);
  endclass

  function int C::f(int b);
    return b;
  endfunction
endmodule
