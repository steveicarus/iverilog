// Check that a localparam of a class specialization that nothing refers to
// is evaluated: the specialization is created after the parameters of the
// design are evaluated. Specializations are made in a module item, in a task,
// through a typedef and through extends.

class C #(int N = 4);
  localparam int M = N * 2;
  int v;
endclass

class D #(int N = 4) extends C #(N + 1);
  localparam int K = N * 3;
endclass

module test;
  C#(8) x;
  typedef C#(2) c2_t;
  c2_t t;
  D#(1) d;
  bit failed = 0;

  task automatic check_task;
    C#(3) y;
    y = new;
    y.v = 3;
    if (y.v != 3) failed = 1;
  endtask

  initial begin
    x = new;
    t = new;
    d = new;
    x.v = 8;
    t.v = 2;
    d.v = 1;
    if (x.v != 8 || t.v != 2 || d.v != 1) failed = 1;
    check_task();

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
