// Check that a later typedef hides an outer function during call lookup.

function integer value;
  value = 1;
endfunction

module test;

  integer result;

  initial result = value();

  typedef logic value;

endmodule
