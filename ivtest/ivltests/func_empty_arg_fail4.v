// Check that an error is reported when passing an empty function argument for a
// port without a default value.

module test;

  function f(integer a, integer b = 2);
    return a + 10 * b + 100 * c;
  endfunction

  initial begin
    integer x;
    x = f( , 3); // This should fail. No default value specified.
    $display("FAILED");
  end

endmodule
