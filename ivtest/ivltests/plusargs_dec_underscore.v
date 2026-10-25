// Check that $value$plusargs with %d handles a value that is only
// underscores without reading outside the argument string.

module test;

  reg [7:0] a, b;
  reg failed;

  initial begin
    failed = 1'b0;
    a = 8'd0;
    b = 8'd0;

    if (!$value$plusargs("a=%d", a) || a !== 8'bx) begin
      $display("FAILED: a = %b", a);
      failed = 1'b1;
    end

    if (!$value$plusargs("b=%d", b) || b !== 8'bx) begin
      $display("FAILED: b = %b", b);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
