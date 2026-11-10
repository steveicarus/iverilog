// Check that an out-of-block definition is a task if and only if the
// extern declaration is a task.

module test;
  class C;
    extern task t();
  endclass

  function void C::t();
  endfunction
endmodule
