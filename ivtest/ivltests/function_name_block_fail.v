// Check that a block name cannot redeclare the enclosing function's name.

module test;

  function integer f(input integer value);
    begin : f // Error: The block name conflicts with the function's name.
    end
  endfunction

  initial $display("FAILED");

endmodule
