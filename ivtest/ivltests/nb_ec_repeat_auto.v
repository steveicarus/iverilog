// Check that non-blocking event control repeat counts can use automatic terms.

module test;

  reg clk;
  reg failed;
  reg [3:0] result;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  task automatic schedule_update;
    input integer count;
    begin
      result <= repeat(count) @(posedge clk) 4'ha;
    end
  endtask

  always #5 clk = ~clk;

  initial begin
    clk = 1'b0;
    failed = 1'b0;
    result = 4'h0;

    schedule_update(2);

    @(posedge clk);
    #1;
    `check(result, 4'h0);

    @(posedge clk);
    #1;
    `check(result, 4'ha);

    if (!failed) begin
      $display("PASSED");
    end
    $finish;
  end

endmodule
