// Check that an error is reported when trying to bind an argument by name that
// is also provided as a positional argument.

module test;

  class C;
    function new(integer a, integer b);
      $display("FAILED");
    endfunction
  endclass

  initial begin
    C c;
    c = new(1, .a(2)); // This should fail. `a` is provided both as a positional
                       // and named argument.
  end

endmodule
