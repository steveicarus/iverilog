// Continuous assignment to an element of a dynamic array or queue is
// illegal (IEEE 1800 6.21).
module test;
  int q[$];
  int d[];
  int x = 1;

  assign q[0] = x;  // error
  assign d[1] = x;  // error
endmodule
