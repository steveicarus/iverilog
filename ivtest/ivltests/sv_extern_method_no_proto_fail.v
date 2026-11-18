// Check that defining a method outside of its class requires an extern
// declaration inside the class.

module test;
  class C;
    function int f();
      return 1;
    endfunction
  endclass

  function int C::f();
    return 2;
  endfunction
endmodule
