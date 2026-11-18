// Check that the class of an out-of-block method definition must exist.

module test;
  function int Missing::f();
    return 1;
  endfunction
endmodule
