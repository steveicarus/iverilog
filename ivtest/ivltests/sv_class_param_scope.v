// Check a parameterized class declared in a module whose parameter default
// depends on a module parameter, and specializations that are first used in
// a task.

module sub #(parameter P = 1);

  class C #(int Q = P);
    function int get();
      return Q;
    endfunction
  endclass

  C c;
  C #(7) c7;
  int got, got7;

  initial begin
    c = new;
    c7 = new;
    got = c.get();
    got7 = c7.get();
  end

endmodule

module test;

  class D #(int N = 1);
    static int count = 0;

    function int get();
      count++;
      return N;
    endfunction
  endclass

  sub #(2) u2();
  sub #(3) u3();
  bit failed = 0;

  task automatic run(output int a, output int b);
    D #(5) d5;
    D #(6) d6;
    d5 = new;
    d6 = new;
    a = d5.get() + d5.get();
    b = d6.get() + d6.count;
  endtask

  initial begin
    int a, b;
    #1;
    if (u2.got != 2 || u2.got7 != 7) failed = 1;
    if (u3.got != 3 || u3.got7 != 7) failed = 1;

    run(a, b);
    if (a != 10 || b != 7) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
