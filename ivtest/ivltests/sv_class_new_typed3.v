// Check that typed constructor calls are supported when the type is a typedef
// of a class type.

module test;

  class C;
    function new;
      $display("PASSED");
    endfunction
  endclass

  typedef C T;

  initial begin
    T c;
    c = T::new;
  end

endmodule
