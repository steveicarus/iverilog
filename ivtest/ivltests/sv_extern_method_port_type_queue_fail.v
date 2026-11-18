// A queue port must not be defined as a dynamic array.

module test;
  class C;
    extern function void f(int a[$]);
  endclass

  function void C::f(int a[]);
  endfunction
endmodule
