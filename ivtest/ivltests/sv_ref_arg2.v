// A write through a ref argument is seen by other processes at once,
// not when the task returns.

module top;
  int v;
  bit failed = 0;

  task automatic slow_set(ref int x);
    x = 1;
    #10;
    x = 2;
    #10;
    x++;
    #10;
  endtask

  initial begin
    v = 0;
    #1;
    slow_set(v);
    if (v !== 3) begin
      $display("FAILED: v=%0d after the call (expect 3)", v);
      failed = 1;
    end
  end

  // The observer must see each write while the task is still running.
  initial begin
    #5;
    if (v !== 1) begin
      $display("FAILED: v=%0d at 5 (expect 1)", v);
      failed = 1;
    end
    #10;
    if (v !== 2) begin
      $display("FAILED: v=%0d at 15 (expect 2)", v);
      failed = 1;
    end
    #10;
    if (v !== 3) begin
      $display("FAILED: v=%0d at 25 (expect 3)", v);
      failed = 1;
    end
  end

  // A write through the argument wakes up a process that waits for the
  // variable.
  int wakeups = 0;
  initial begin
    forever begin
      @(v);
      wakeups++;
    end
  end

  initial begin
    #40;
    if (wakeups !== 3) begin
      $display("FAILED: wakeups=%0d (expect 3)", wakeups);
      failed = 1;
    end
    if (!failed) $display("PASSED");
  end
endmodule
