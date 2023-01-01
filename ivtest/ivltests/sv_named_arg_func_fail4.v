// Check that an error is reported trying to provide a positional argument to a
// function after a named argument.

module test;

  function f(integer a, integer b);
    $display("FAILED");
  endfunction

  initial begin
    integer x;
    x = f(.a(2), 1); // This should fail. Positional arguments must precede
                     // named arguments.
  end

endmodule
