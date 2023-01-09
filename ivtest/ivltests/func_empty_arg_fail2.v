// Check that passing additional empty arguments to a function results in an error.

module test;

  function f(integer a = 1, integer b = 2, integer c = 3);
    return a + 10 * b + 100 * c;
  endfunction

  initial begin
    integer x;
    x = f(4, 5, 6, ); // This should fail. 4 empty args, even though the function
                      // only takes 3 args
    $display("FAILED");
  end

endmodule
