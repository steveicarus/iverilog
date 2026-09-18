// Check that a later named event in the local scope hides a compilation-unit
// function used as a statement.

function void value;
endfunction

module test;

  initial value();

  event value;

endmodule
