// Procedural continuous assignment to an element of a dynamic array or
// queue is illegal (IEEE 1800 6.21).
module test;
  int q[$];
  int x = 1;

  initial assign q[0] = x;  // error
endmodule
