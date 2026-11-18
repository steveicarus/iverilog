// Check class scope resolution of the parameters, localparams, typedefs and
// static members of a parameterized class: C#(8)::N, C::N, C#(T)::name.

class C #(int N = 4, type T = int);
  localparam int M = N * 2;
  typedef T elem_t;
  static int count = 5;

endclass

module test;

  localparam int A = C#(8)::N;
  localparam int B = C::N;
  localparam int D = C#(8)::M;
  localparam int E = C::M;

  C#(8, logic [3:0])::elem_t v4;
  C#(.T(shortint))::elem_t s;
  C::elem_t i;

  localparam C#(8, logic [3:0])::elem_t LP = 5;
  parameter C#(.T(shortint))::elem_t PP = 300;

  logic [C#(6)::N-1:0] vec;
  C#(8) d = new;
  bit failed = 0;

  initial begin
    if (A != 8 || B != 4 || D != 16 || E != 8) failed = 1;
    if ($bits(v4) != 4 || $bits(s) != 16 || $bits(i) != 32) failed = 1;
    if ($bits(vec) != 6) failed = 1;
    if (LP != 5 || PP != 300 || $bits(PP) != 16) failed = 1;

    if (C#(8)::count != 5) failed = 1;
    // Each specialization has its own static.
    d.count = 7;
    if (C#(8)::count != 7 || C::count != 5 || C#(9)::count != 5) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
