// Check that disable skips side effects in a do-while condition.

module test;

  bit failed = 1'b0;
  integer stop;

  function integer run(input integer stop);
    integer i;
    i = 0;
    begin : body
      do begin
        if (stop) disable body;
      end while (i++ < 10);
    end
    run = i;
  endfunction

  localparam integer stopped = run(1);

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(stopped, 0);
    stop = 1;
    `check(run(stop), 0);
    stop = 0;
    `check(run(stop), 11);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
