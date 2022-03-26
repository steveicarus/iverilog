// Check that a class constructor with non-ANSI port results in an error.

module test;

  class C;
    function new;
      input x; // This is a syntax error
      $display("FAILED");
    endfunction
  endclass

  C c = new;

endmodule
