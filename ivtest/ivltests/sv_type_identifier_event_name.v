// Check that event names can shadow visible type identifiers.

typedef int T;

module test;

  reg failed;
  reg seen;

  event T;

  initial begin
    failed = 1'b0;
    seen = 1'b0;

    #1 -> T;
    #1;

    if (seen !== 1'b1) begin
      $display("FAILED(%0d). Event T was not triggered", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

  always @T seen = 1'b1;

endmodule
