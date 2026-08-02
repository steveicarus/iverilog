// Check that $unit:: does not allow a forward named event reference.

module test;

  initial -> $unit::value;

endmodule

event value;
