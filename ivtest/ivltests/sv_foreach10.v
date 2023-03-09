// Check that foreach counts from $left to $right for dynamic arrays and queues.

module test;

  logic d[];
  logic q[$];

  initial begin
    bit failed;
    int exp_idx;

    failed = 1'b0;

    // Dynamic arrays and queues always count from 0 to $size - 1
    d = '{0, 0};
    exp_idx = 0;
    foreach(d[idx]) begin
      if (idx !== exp_idx) begin
        $display("FAILED: Expected %0d, got %0d", exp_idx, idx);
        failed = 1'b1;
      end
      exp_idx++;
    end

    q = '{0, 0, 0};
    exp_idx = 0;
    foreach(q[idx]) begin
      if (idx !== exp_idx) begin
        $display("FAILED: Expected %0d, got %0d", exp_idx, idx);
        failed = 1'b1;
      end
      exp_idx++;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
