// Check that $unit:: does not allow a forward parameter reference.

module test;

  localparam integer result = $unit::value;

endmodule

parameter integer value = 42;
