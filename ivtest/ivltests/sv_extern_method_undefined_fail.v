// Check that an extern class method must be defined.

module test;
  class C;
    extern function int f();
  endclass

  C c = new;
endmodule
