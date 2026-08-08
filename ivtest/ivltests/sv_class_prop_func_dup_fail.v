// Check that a class property and function cannot have the same name.

module test;

  class C;
    int value;

    function int value;
      return 0;
    endfunction
  endclass

endmodule
