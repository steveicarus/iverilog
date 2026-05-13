// Check that `super` access in a class without a parent class is rejected.

module test;

  class C;
    function void f;
      super.x = 1;
    endfunction
  endclass

endmodule
