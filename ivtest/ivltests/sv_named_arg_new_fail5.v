// Check that an error is reported when binding an empty value to an argument by
// name and the argument does not have a default value.

module test;

  class C;
    function new(integer a);
      $display("FAILED");
    endfunction
  endclass

  initial begin
    C c;
    c = new(.a()); // This should fail. `a` has no default value.
  end

endmodule
