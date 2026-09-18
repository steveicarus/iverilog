// Check that a later local variable hides a compilation-unit function used
// as a statement.

function integer value;
  value = 0;
endfunction

module test;

  initial value();

  integer value;

endmodule
