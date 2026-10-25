// Check that a bounded string queue evaluates discarded pattern elements.

module test;
  string q[$:1];
  integer calls = 0;
  bit failed = 1'b0;

  function automatic string touch;
    calls = calls + 1;
    return "discarded";
  endfunction

  initial begin
    q = '{"left", "right", touch(), touch()};
    if (calls != 2 || q.size() != 2 || q[0] != "left" || q[1] != "right") begin
      $display("FAILED: pattern calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    q = {"left", "right", touch()};
    if (calls != 3 || q.size() != 2 || q[0] != "left" || q[1] != "right") begin
      $display("FAILED: concatenation calls=%0d size=%0d", calls, q.size());
      failed = 1'b1;
    end
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
