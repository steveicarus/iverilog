// Check that a function name can invalidate an earlier property type.

typedef int T;

class C;
  T value;

  function T;
    T = 0;
  endfunction
endclass

module test;
endmodule
