// Check that an out-of-block method definition inside a class is an error.
module test;
  class C;
    extern function int f();
    function int C::f(); return 1; endfunction
  endclass
endmodule
