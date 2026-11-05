// Check that a bounded integer queue evaluates discarded pattern elements.

module test;
  integer q[$:1];
  integer calls = 0;
  bit failed = 1'b0;

  function automatic integer touch;
    calls = calls + 1;
    return 33;
  endfunction

  initial begin
    q = '{11, 22, touch(), touch()};
    if (calls !== 2 || q.size() !== 2 || q[0] !== 11 || q[1] !== 22) begin
      $display("FAILED: pattern calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    q = {11, 22, touch()};
    if (calls !== 3 || q.size() !== 2 || q[0] !== 11 || q[1] !== 22) begin
      $display("FAILED: concatenation calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
