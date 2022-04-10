// Check that a class constructor declaration without parenthesis after the
// `new` is supported.

module test;

  class C;
    function new;
      $display("PASSED");
    endfunction
  endclass

  C c = new;

endmodule
