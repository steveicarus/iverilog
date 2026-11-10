// A syntax error in an extern prototype must be reported as an
// ordinary syntax error.

module test;
  class C;
    extern function int f() int x;
  endclass
endmodule
