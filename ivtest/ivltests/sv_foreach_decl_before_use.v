// Check that foreach skips declarations that are after the array use.

module test;

  reg failed;
  reg [31:0] A[0:1];

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  generate
    begin : inner
      initial begin
        integer count;

        failed = 1'b0;
        count = 0;

        foreach (A[i]) begin
          count = count + 1;
        end

        `check(count, 2);

        if (!failed) begin
          $display("PASSED");
        end
      end

      reg [31:0] A[0:3];
    end
  endgenerate

endmodule
