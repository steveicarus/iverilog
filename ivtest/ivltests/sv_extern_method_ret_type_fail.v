// The definition must use the return type of the prototype.

module test;
  class C;
    extern function int f(int a);
  endclass

  function void C::f(int a);
  endfunction
endmodule
