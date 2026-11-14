// A class scope in the name of an extern prototype is a syntax error.

module test;
  class C;
    extern task C::t();
  endclass
endmodule
