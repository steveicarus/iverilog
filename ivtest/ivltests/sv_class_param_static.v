// Check that each specialization of a parameterized class is a type of its
// own with its own static properties, and that specializations with the same
// parameter values are the same type. Static initializers of a
// specialization run before the initial blocks of the design.

class C #(int N = 4);
  static int count = 0;
  static int init = N * 2 + 1;

  function new();
    count++;
  endfunction
endclass

module test;

  C a1, a2;
  C #(4) b;
  C #(.N(2 + 2)) c;
  C #(8) d1, d2;
  C #(16) e;
  bit failed = 0;

  initial begin
    if (a1.init != 9 || d1.init != 17 || e.init != 33) failed = 1;

    a1 = new;
    b = new;
    c = new;
    d1 = new;
    d2 = new;
    e = new;

    // C, C#(4) and C#(2+2) are the same type.
    a2 = b;
    a2 = c;
    b = a1;

    // Same parameters, same type.
    d1 = d2;

    if (a1.count != 3) failed = 1;
    if (d1.count != 2) failed = 1;
    if (e.count != 1) failed = 1;

    d1.count = 10;
    if (d2.count != 10 || a1.count != 3 || e.count != 1) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
