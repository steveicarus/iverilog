// Check that an error is reported when trying to bind the same argument by name
// multiple times.

module test;

  class B;
    function new(integer a, integer b);
      $display("FAILED");
    endfunction
  endclass

  class C extends B(.a(1), .a(2)); // This should fail. `a` is provided twice
                                   // as a named argument.
  endclass

  initial begin
    C c;
    c = new;
  end

endmodule
