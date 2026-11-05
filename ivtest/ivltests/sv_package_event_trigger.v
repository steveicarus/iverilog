// Check that package-scoped events are supported.

package p;
  event e;
endpackage

module test;

  integer count;
  reg failed;

  always @p::e begin
    count = count + 1;
  end

  initial begin
    count = 0;
    failed = 1'b0;
    #1;
    // The trigger must be the direct if body to expose a separate semicolon.
    if (1) -> p::e;
    else begin
      failed = 1'b1;
    end
    #1;
    if (0) -> p::e;
    else begin
      if (count !== 1) begin
        failed = 1'b1;
      end
    end
    #1;
    if (failed || count !== 1) begin
      $display("FAILED: count=%0d", count);
    end else begin
      $display("PASSED");
    end
  end

endmodule
