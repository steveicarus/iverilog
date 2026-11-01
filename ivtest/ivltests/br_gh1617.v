// Check that writing to the last element of a queue (q[$]) works.
module test;
  int q[$];
  int e[$];
  bit failed = 1'b0;

  initial begin
    q = '{1, 2, 3};
    q[$] = 30;
    if (q.size() != 3 || q[0] != 1 || q[1] != 2 || q[2] != 30) begin
      $display("FAILED: q = %p", q);
      failed = 1'b1;
    end

    q.push_back(4);
    q[$] = 40;
    if (q.size() != 4 || q[2] != 30 || q[3] != 40) begin
      $display("FAILED: q = %p", q);
      failed = 1'b1;
    end

    q[$] += 1;
    if (q.size() != 4 || q[3] != 41) begin
      $display("FAILED: q = %p", q);
      failed = 1'b1;
    end

    // Writing to the last element of an empty queue is ignored.
    e[$] = 5;
    if (e.size() != 0) begin
      $display("FAILED: e.size() = %0d", e.size());
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
