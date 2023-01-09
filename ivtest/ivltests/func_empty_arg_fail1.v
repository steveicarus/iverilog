// Check that passing too many empty arguments to a function results in an error.

module test;

  function f(integer a = 1, integer b = 2, integer c = 3);
    return a + 10 * b + 100 * c;
  endfunction

  initial begin
    integer x;
    x = f( ,  ,  ,); // This should fail. 4 empty args, even though the function
                     // only takes 3 args
    $display("FAILED");
  end

endmodule
