// Check discarded pattern evaluation when assigning a bounded queue property.

module test;
  class C;
    int q[$:1];
  endclass

  C c;
  integer calls = 0;

  function automatic int touch;
    calls = calls + 1;
    return 33;
  endfunction

  initial begin
    c = new;
    c.q = '{11, 22, touch()};
    if (calls == 1 && c.q.size() == 2 && c.q[0] == 11 && c.q[1] == 22) begin
      $display("PASSED");
    end else begin
      $display("FAILED: calls=%0d size=%0d", calls, c.q.size());
    end
  end
endmodule
