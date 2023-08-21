// Check that an error is reported when trying to bind an argument by name that
// is also provided as a positional argument.

module test;

  class B;
    function new(integer a, integer b);
      $display("FAILED");
    endfunction
  endclass

  class C extends B(1, .a(2)); // This should fail. `a` is provided both as a
                               // positional and named argument.
  endclass

  initial begin
    C c;
    c = new;
  end

endmodule
