// Check that a function name can invalidate an earlier constructor type.

class C;
endclass

typedef C T;

module test;

  C value;

  initial value = T::new;

  function T;
    T = 0;
  endfunction

endmodule
