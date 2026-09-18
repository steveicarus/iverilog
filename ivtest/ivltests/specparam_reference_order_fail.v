// Check that a module-body specparam is declared before it is referenced.

module test;

  wire value;
  assign value = delay;
  specparam delay = 1;

endmodule
