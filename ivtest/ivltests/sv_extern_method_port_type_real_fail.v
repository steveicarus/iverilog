// A real port must not be defined as a shortreal.

module test;
  class C;
    extern function void f(real a);
  endclass

  function void C::f(shortreal a);
  endfunction
endmodule
