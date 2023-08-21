// Check that an error is reported when trying to bind an argument by nae that
// does not exist

module test;

  function f(integer a, integer b);
    $display("FAILED");
  endfunction

  initial begin
    integer x;
    x = f(.b(2), .c(1)); // This should fail. `c` is not an arugment of `f`.
  end

endmodule
