// A static method or a static initializer that uses a class scope with a
// changing value would make specializations forever. Check that it is
// reported once.

class C #(int N = 1);
  static function int g();
    return N;
  endfunction

  static function int f();
    return C#(N+1)::g();
  endfunction
endclass

module test;
  initial $display("FAILED %0d", C#(1)::f());
endmodule
