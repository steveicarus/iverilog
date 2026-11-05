// Check that a bounded real queue evaluates discarded pattern elements.

module test;
  real q[$:1];
  integer calls = 0;
  bit failed = 1'b0;

  function automatic real touch;
    calls = calls + 1;
    return 3.5;
  endfunction

  initial begin
    q = '{1.5, 2.5, touch(), touch()};
    if (calls != 2 || q.size() != 2 || q[0] != 1.5 || q[1] != 2.5) begin
      $display("FAILED: pattern calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    q = {1.5, 2.5, touch()};
    if (calls != 3 || q.size() != 2 || q[0] != 1.5 || q[1] != 2.5) begin
      $display("FAILED: concatenation calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
