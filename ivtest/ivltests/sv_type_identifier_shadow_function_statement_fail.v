// Check that a later typedef hides a compilation-unit function used as a
// statement.

function integer value;
  value = 0;
endfunction

module test;

  initial value();

  typedef logic value;

endmodule
