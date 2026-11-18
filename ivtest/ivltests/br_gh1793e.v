// Check output arguments of an automatic task call that are elements of
// dynamic arrays, queues and strings with a constant index in an automatic
// calling task. The container is a variable of the calling task, which must
// not be read from the context of the called task.

module test;

  reg failed;

  task automatic put_int(input int a, output int v);
    v = a;
  endtask

  task automatic put_real(input real a, output real v);
    v = a;
  endtask

  task automatic put_byte(input byte a, output byte v);
    v = a;
  endtask

  task automatic run();
    int dq[];
    int qq[$];
    real rq[$];
    byte bq[$];
    string s;

    dq = new[3];
    qq = '{0, 0, 0};
    rq = '{0.0, 0.0};
    bq = '{0, 0};
    s = "abc";

    put_int(7, dq[2]);
    put_int(8, qq[2]);
    put_real(2.5, rq[1]);
    put_byte(3, bq[1]);
    put_byte(8'h5a, s[2]);

    if (dq[0] !== 0 || dq[1] !== 0 || dq[2] !== 7) begin
      $display("FAILED: dynamic array is %p", dq);
      failed = 1'b1;
    end
    if (qq[0] !== 0 || qq[1] !== 0 || qq[2] !== 8) begin
      $display("FAILED: queue is %p", qq);
      failed = 1'b1;
    end
    if (rq[0] != 0.0 || rq[1] != 2.5) begin
      $display("FAILED: real queue is %p", rq);
      failed = 1'b1;
    end
    if (bq[0] !== 0 || bq[1] !== 3) begin
      $display("FAILED: byte queue is %p", bq);
      failed = 1'b1;
    end
    if (s != "abZ") begin
      $display("FAILED: string is %s", s);
      failed = 1'b1;
    end
  endtask

  initial begin
    failed = 1'b0;
    run();
    if (!failed) $display("PASSED");
  end

endmodule
