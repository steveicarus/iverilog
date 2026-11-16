// Check calling static methods through a class scope, P::f() and C#(8)::f(),
// and writing static properties through it, as statements and in expressions.

class P;
  static int total = 10;
  static function int add(int a);
    return a + total;
  endfunction
  static task bump(int a);
    total += a;
  endtask
  static function void reset();
    total = 10;
  endfunction
endclass

class C #(int N = 4, type T = int);
  localparam int M = N * 2;
  static int count = 0;

  static function int width();
    return N;
  endfunction

  static function int twice();
    return 2 * width();
  endfunction

  static function T conv(int v);
    return T'(v);
  endfunction

  static task incr(int by = 1);
    count += by;
  endtask

  static function void bump1();
    count += 1;
  endfunction

  static task bump_twice();
    bump1();
    incr();
  endtask

  function int inst_width();
    return width() + M;
  endfunction
endclass

module test;

  bit failed = 0;
  int x;
  C#(8) h = new;

  initial begin
    // Not parameterized.
    if (P::add(5) != 15) failed = 1;
    P::bump(2);
    if (P::total != 12) failed = 1;
    x = P::add(1) + P::add(2);
    if (x != 27) failed = 1;
    P::reset();
    if (P::total != 10) failed = 1;

    // Parameters come from the specialization named in the scope.
    if (C#(8)::width() != 8) failed = 1;
    if (C::width() != 4) failed = 1;
    if (C#(3)::twice() != 6) failed = 1;
    if (C#(.N(5))::width() != 5) failed = 1;
    if (C#(8, logic [3:0])::conv(21) != 5) failed = 1;
    if ($bits(C#(8, byte)::conv(1)) != 8) failed = 1;
    if (C#(8)::width() + C::width() != 12) failed = 1;

    // Each specialization has its own static properties.
    C#(8)::count = 1;
    if (C#(8)::count != 1 || C::count != 0 || h.count != 1) failed = 1;
    C::count++;
    ++C::count;
    C#(8)::count--;
    if (C::count != 2 || C#(8)::count != 0) failed = 1;
    C#(8)::count += 5;
    C::count <= 7;
    #1;
    if (C#(8)::count != 5 || C::count != 7) failed = 1;

    C#(8)::incr();
    C#(8)::incr(3);
    C::incr(.by(2));
    if (C#(8)::count != 9 || C::count != 9) failed = 1;

    // A static method calls another one without a scope.
    C#(8)::bump_twice();
    if (C#(8)::count != 11) failed = 1;

    // An object still reaches a static method.
    if (h.width() != 8 || h.inst_width() != 24) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
