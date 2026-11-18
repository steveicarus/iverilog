// A typedef in the prototype matches the spelled-out type in the definition.

module test;
  typedef logic [7:0] byte_t;

  class A;
    int v = 7;
  endclass
  typedef A a_t;
  typedef bit signed [3:0] nib_t;

  class C;
    extern function byte_t f(byte_t a, int m[2], string s, real r);
    extern function A g(a_t x);
    extern function nib_t h(bit signed [3:0] x);
  endclass

  function logic [7:0] C::f(logic [7:0] a, int m[2], string s, real r);
    return a + m[1];
  endfunction

  function a_t C::g(A x);
    return x;
  endfunction

  function bit signed [3:0] C::h(nib_t x);
    return x;
  endfunction

  C c;
  A a, b;
  int m[2] = '{1, 2};
  initial begin
    c = new;
    a = new;
    b = c.g(a);
    if (c.f(8'd5, m, "s", 1.0) === 8'd7 && c.h(-3) === -3 && b.v == 7)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
