// Check that passing empty arguments to a function without any ports is an
// error.

module test;

  function f();
    return 42;
  endfunction

  initial begin
    integer x;
    x = f( , ); // This should fail. The function takes no arguments.
    $display("FAILED");
  end

endmodule
