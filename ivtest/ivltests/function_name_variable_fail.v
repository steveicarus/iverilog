// Check that a variable cannot redeclare the enclosing function's name.

module test;

  function integer f(input integer value);
    integer f; // Error: f is already the function's return variable.
    f = value;
  endfunction

  initial $display("FAILED");

endmodule
