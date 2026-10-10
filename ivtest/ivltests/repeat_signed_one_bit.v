// Check that signed one-bit repeat counts are evaluated once without looping.

module test;

  reg signed count;
  reg failed;
  integer calls;

  function signed f(input value);
    begin
      calls = calls + 1;
      f = value;
    end
  endfunction

  `define check(val) \
    count = val; \
    calls = 0; \
    repeat (f(count)) begin \
      $display("FAILED(%0d): signed repeat (%s) executed", \
               `__LINE__, `"val`"); \
      failed = 1'b1; \
      disable check_counts; \
    end \
    if (calls !== 1) begin \
      $display("FAILED(%0d): repeat (%s) evaluated count %0d times", \
               `__LINE__, `"val`", calls); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    begin : check_counts
      `check(1'b0);
      `check(1'b1);
      `check(1'bx);
      `check(1'bz);
    end
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
