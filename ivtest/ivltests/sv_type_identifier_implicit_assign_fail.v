// Check that a type identifier does not become an implicit net on the left
// side of a continuous assignment.

module test;
  typedef logic value;
  assign value = 1'b1;
endmodule
