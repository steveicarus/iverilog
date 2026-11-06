// Force of an element of a dynamic array or queue is illegal
// (IEEE 1800 6.21).
module test;
  int d[];
  int x = 1;

  initial force d[0] = x;  // error
endmodule
