module top;
  reg passed;
  int q[$];

  initial begin
    passed = 1'b1;
    q.push_front(10);
    q.pop_back(); // This should emit a warning
    if (q.size() != 0) begin
      $display("FAILED: pop_back() did not pop value when called as a task.");
      passed = 1'b0;
    end

    q.delete();
    q.push_front(20);
    q.pop_front(); // This should emit a warning
    if (q.size() != 0) begin
      $display("FAILED: pop_front() did not pop value when called as a task.");
      passed = 1'b0;
    end

    q.size();

    if (passed) $display("PASSED");
  end
endmodule
