// Check that a typedef cannot use the enclosing function's name.

module test;

  function integer f(input integer value);
    typedef integer f; // Error: f conflicts with the enclosing function's name.
    return value;
  endfunction

  initial $display("FAILED");

endmodule
