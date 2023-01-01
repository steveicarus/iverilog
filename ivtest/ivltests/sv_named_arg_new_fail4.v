// Check that an error is reported trying to provide a positional argument to a
// function after a named argument.

module test;

  class C;
    function new(integer a, integer b);
      $display("FAILED");
    endfunction
  endclass

  initial begin
    C c;
    c = new(.a(2), 1); // This should fail. Positional arguments must precede
                       // named arguments.
  end

endmodule
