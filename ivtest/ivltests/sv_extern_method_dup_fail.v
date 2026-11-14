// Check that an extern class method cannot be defined twice.

module test;
  class C;
    extern function int f();
  endclass

  function int C::f();
    return 1;
  endfunction

  function int C::f();
    return 2;
  endfunction
endmodule
