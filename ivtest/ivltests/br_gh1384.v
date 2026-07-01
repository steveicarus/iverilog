// Check that classes declared inside generate blocks can be used.

module test;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  if (1) begin : g
    class C;
      int value;

      function new(int value);
        this.value = value;
      endfunction
    endclass

    C c = new(42);
  end

  for (genvar i = 0; i < 2; i = i + 1) begin : h
    class C;
      int value;

      function new(int value);
        this.value = value + i;
      endfunction
    endclass

    C c = new(10);
  end

  initial begin
    failed = 1'b0;

    `check(g.c.value, 42);
    `check(h[0].c.value, 10);
    `check(h[1].c.value, 11);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
