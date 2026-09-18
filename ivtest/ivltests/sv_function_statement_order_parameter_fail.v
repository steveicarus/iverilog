// Check that a later parameter in the local scope hides a compilation-unit
// function used as a statement.

function integer value;
  value = 0;
endfunction

module test;

  initial value();

  parameter value = 1;

endmodule
