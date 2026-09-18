// Check that loop labels name their implicit scopes in hierarchical paths.

module test;

  reg failed;
  reg [255:0] scope;
  reg [7:0] values [0:1];

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    FOR: for (int i = 0; i < 2; i = i + 1)
      FOREACH: foreach (values[j]) begin : INNER
        // Use a static body variable instead of the automatic loop indices.
        static reg [7:0] value;

        value = 2*i + j;
        $sformat(scope, "%m");
      end

    `check(scope, "test.FOR.FOREACH.INNER");
    `check(test.FOR.FOREACH.INNER.value, 3);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
