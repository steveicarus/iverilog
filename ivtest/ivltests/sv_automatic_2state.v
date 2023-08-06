// Check that automatic 2-state variables are initialized to 0.

module test;

  bit failed = 1'b0;

  function automatic int f(int x);
    int a;
    if (a !== 0) begin
      failed = 1'b1;
    end
    return x;
  endfunction

  task automatic t;
    int a;
    if (a !== 0) begin
      failed = 1'b1;
    end
  endtask

  initial begin
    int x;

    t;
    x = f(10);

    if (failed) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
