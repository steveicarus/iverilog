// Check that typed constructor calls are supported.

module test;

  class C;
    function new;
      $display("PASSED");
    endfunction
  endclass

  initial begin
    C c;
    c = C::new;
  end

endmodule
