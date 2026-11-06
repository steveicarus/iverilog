// Check that a call of a virtual task runs the override of the class of
// the object, including delays and events in the override, output and
// inout arguments, calls from another task and from fork branches, and
// super.task().

module test;

  event ev;

  class Base;
    int log;
    virtual task t(input int a, output int b, inout int c);
      b = a;
      c = c + 1;
      log = log * 10 + 1;
    endtask
    virtual task wait_it();
      #1;
      log = log * 10 + 1;
    endtask
    task call_t(output int r);
      int tmp = 5;
      t(1, r, tmp);
    endtask
  endclass

  class Mid extends Base;
    task t(input int a, output int b, inout int c);
      #3;
      b = a * 2;
      c = c + 10;
      log = log * 10 + 2;
    endtask
    task wait_it();
      @(ev);
      log = log * 10 + 2;
    endtask
  endclass

  class Leaf extends Mid;
    task t(input int a, output int b, inout int c);
      super.t(a, b, c);
      b = b + 1;
      log = log * 10 + 3;
    endtask
  endclass

  class Plain extends Base;
  endclass

  Base b;
  Mid m;
  Leaf l;
  Plain p;
  int out, io, errors = 0;
  time t0;

  initial begin
    b = new;
    io = 0;
    b.t(4, out, io);
    if (out != 4 || io != 1 || $time != 0) errors += 1;

    m = new;
    b = m;
    io = 0;
    b.t(4, out, io);
    if (out != 8 || io != 10 || $time != 3) errors += 1;
    // The class of the handle decides if the task is not virtual
    m.t(1, out, io);
    if (out != 2 || io != 20 || $time != 6) errors += 1;

    l = new;
    b = l;
    io = 0;
    b.t(4, out, io);
    if (out != 9 || io != 10 || $time != 9 || l.log != 23) errors += 1;

    p = new;
    b = p;
    io = 0;
    b.t(4, out, io);
    if (out != 4 || io != 1) errors += 1;

    // Called from another task of the base class
    b = l;
    b.call_t(out);
    if (out != 3 || $time != 12) errors += 1;

    // An event in the override
    b = m;
    m.log = 0;
    fork
      begin
        b.wait_it();
        if ($time != 14) errors += 1;
      end
      begin
        #2 ->ev;
      end
    join
    if (m.log != 2) errors += 1;

    // Disable fork stops a dispatched task
    m.log = 0;
    fork
      b.wait_it();
    join_none
    #1;
    disable fork;
    #1 ->ev;
    #1;
    if (m.log != 0) begin
      $display("log=%0d", m.log);
      errors += 1;
    end

    if (errors == 0)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
