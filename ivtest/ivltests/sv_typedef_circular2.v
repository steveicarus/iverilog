// Check that longer chains of circular type definitions are detected as an
// error.

module test;

  typedef T1;

  typedef struct packed {
    T1 x;
  } T2;

  typedef T2 [1:0] T3;

  typedef T3 T1;

  T1 x;

endmodule
