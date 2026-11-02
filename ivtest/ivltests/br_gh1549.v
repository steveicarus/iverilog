// Check that a queue can be passed to a function argument of queue type,
// and that the function gets its own copy.
module top;
  function automatic int first(input int q[$]);
    return q[0];
  endfunction

  function automatic int grow(input int a, input int q[$], input int b);
    q.push_back(9);
    return a + b + q.size();
  endfunction

  int q[$];
  bit failed = 1'b0;

  initial begin
    q = {5, 3, 8};

    if (first(q) !== 5) begin
      $display("FAILED: first(q) = %0d, expected 5", first(q));
      failed = 1'b1;
    end

    if (grow(1, q, 2) !== 7) begin
      $display("FAILED: grow() returned wrong value");
      failed = 1'b1;
    end

    if (q.size() != 3) begin
      $display("FAILED: caller queue size is %0d, expected 3", q.size());
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
