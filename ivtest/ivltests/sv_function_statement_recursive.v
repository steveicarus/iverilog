// Check that a recursive non-void function can call itself as a statement.

module test;

  integer calls;

  function automatic integer recurse(input integer count);
    calls = calls + 1;
    if (count > 0) begin
      recurse(count - 1);
    end
    recurse = 0;
  endfunction

  initial begin
    calls = 0;
    recurse(2);

    if (calls == 3) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
