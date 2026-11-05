// Nonblocking assignment to an element of a dynamic array or queue is
// illegal (IEEE 1800 10.4.2).
module test;
  int q[$];
  int d[];
  int i = 0;

  initial begin
    q[i] <= 1;        // error
    q[$] <= 2;        // error
    d[i] <= 3;        // error
    q[0] <= #1 4;     // error
    d[1] <= #1 5;     // error
  end
endmodule
