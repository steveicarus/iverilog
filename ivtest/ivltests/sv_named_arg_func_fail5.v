// Check that an error is reported when binding an empty value to an argument by
// name and the argument does not have a default value.

module test;

  function f(integer a);
    $display("FAILED");
  endfunction

  initial begin
    integer x;
    x = f(.a()); // This should fail. `a` has no default value.
  end

endmodule
