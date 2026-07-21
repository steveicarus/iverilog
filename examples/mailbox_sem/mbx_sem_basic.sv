// Tier A #5 smoke: built-in mailbox #(int) + semaphore API.
module mbx_sem_basic;
  mailbox #(int) mb = new();
  semaphore sem = new(1);
  int v, n, ok;
  int pass;

  initial begin
    pass = 1;

    mb.put(7);
    mb.get(v);
    if (v !== 7) begin
      $display("FAIL: mb.get got %0d want 7", v);
      pass = 0;
    end

    ok = mb.try_put(8);
    if (ok !== 1) begin
      $display("FAIL: try_put expected 1 got %0d", ok);
      pass = 0;
    end

    ok = mb.try_get(v);
    if (ok !== 1 || v !== 8) begin
      $display("FAIL: try_get ok=%0d v=%0d", ok, v);
      pass = 0;
    end

    n = mb.num();
    if (n !== 0) begin
      $display("FAIL: num expected 0 got %0d", n);
      pass = 0;
    end

    ok = mb.try_get(v);
    if (ok !== 0) begin
      $display("FAIL: empty try_get expected 0 got %0d", ok);
      pass = 0;
    end

    sem.get(1);
    sem.put(1);
    ok = sem.try_get(1);
    if (ok !== 1) begin
      $display("FAIL: sem.try_get expected 1 got %0d", ok);
      pass = 0;
    end
    ok = sem.try_get(1);
    if (ok !== 0) begin
      $display("FAIL: sem.try_get empty expected 0 got %0d", ok);
      pass = 0;
    end

    fork
      begin
        #5;
        mb.put(42);
      end
      begin
        mb.get(v);
        if (v !== 42) begin
          $display("FAIL: blocking get got %0d", v);
          pass = 0;
        end
      end
    join

    fork
      begin
        #5;
        sem.put(1);
      end
      begin
        sem.get(1);
      end
    join

    if (pass) $display("PASSED");
    else $display("FAILED");
  end
endmodule
