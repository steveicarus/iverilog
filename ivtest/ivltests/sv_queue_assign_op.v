// Check that assignment operators are supported on queue elements.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  integer x[$];
  integer i;

  initial begin
    x = '{0, 1};
    // Static index
    x[1] += 5;
    `check(x[1], 6);
    x[1] -= 2;
    `check(x[1], 4);
    x[1] *= 25;
    `check(x[1], 100);
    x[1] /= 5;
    `check(x[1], 20);
    x[1] %= 3;
    `check(x[1], 2);

    x[1] = 'haa;
    x[1] &= 'h33;
    `check(x[1], 'h22);
    x[1] |= 'h11;
    `check(x[1], 'h33);
    x[1] ^= 'h22;
    `check(x[1], 'h11);

    x[1] <<= 3;
    `check(x[1], 'h88);
    x[1] <<<= 1;
    `check(x[1], 'h110);
    x[1] >>= 2;
    `check(x[1], 'h44);
    x[1] >>>= 1;
    `check(x[1], 'h22);

    // Dynamic index
    x[1] = 1;
    i = 1;
    x[i] += 5;
    `check(x[i], 6);
    x[i] -= 2;
    `check(x[i], 4);
    x[i] *= 25;
    `check(x[i], 100);
    x[i] /= 5;
    `check(x[i], 20);
    x[i] %= 3;
    `check(x[i], 2);

    x[i] = 'haa;
    x[i] &= 'h33;
    `check(x[i], 'h22);
    x[i] |= 'h11;
    `check(x[i], 'h33);
    x[i] ^= 'h22;
    `check(x[i], 'h11);

    x[i] <<= 3;
    `check(x[i], 'h88);
    x[i] <<<= 1;
    `check(x[i], 'h110);
    x[i] >>= 2;
    `check(x[i], 'h44);
    x[i] >>>= 1;
    `check(x[i], 'h22);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
