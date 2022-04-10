// Check that it is possible to explicitly call the base class constructor, even
// if it does not take any parameters. Check that it is possible to call it
// without parenthesis after the `new`.

module test;

  class B;
    function new();
      $display("PASSED");
    endfunction
  endclass

  class C extends B;
    function new();
      super.new;
    endfunction
  endclass

  C c = new;

endmodule
