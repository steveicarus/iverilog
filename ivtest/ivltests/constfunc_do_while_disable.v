// Check that disabling a constant-function block skips the do-while condition.

module test;

  bit failed = 1'b0;

  function integer condition(input integer value);
    condition = value;
  endfunction

  function integer run(input integer stop);
    integer count;
    begin : body
      count = 0;
      run = 7;
      do begin
        count = count + 1;
        if (stop) disable body;
      end while (condition(count < 2));
      run = count;
    end
  endfunction

  localparam integer stopped = run(1);
  localparam integer completed = run(0);

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(stopped, 7);
    `check(completed, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
