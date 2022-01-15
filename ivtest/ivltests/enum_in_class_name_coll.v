// Check that the enum names are added to the lexor scope of the class and
// name collisions with other symbols are reported as errors.

module test;

class C;
  enum {
    A = 10
  } e;
  typedef int A;
endclass

endmodule
