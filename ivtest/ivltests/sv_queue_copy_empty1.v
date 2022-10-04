// Check that it is possible to copy an empty queue.

module test;

  typedef int T[$];

  initial begin
    T q1;
    T q2;
    q1 = '{1, 2, 3};
    q1 = q2;
    if (q1.size() == 0 && q2.size() == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
