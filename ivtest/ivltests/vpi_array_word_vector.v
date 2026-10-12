// Check four-state VPI vector writes across word boundaries in arrays and queues.

module test;

  bit failed = 1'b0;
  reg [64:0] da [];
  reg [64:0] q [$];
  bit [64:0] bits_da [];
  bit [64:0] bits_q [$];
  reg [64:0] fixed_array [0:0];
  reg [64:0] value;
  integer i;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define write(val, bits) \
    value = val; \
    $put_array_vector("test.da", 1, value); \
    `check(da[1], value); \
    $put_array_vector("test.q", 1, value); \
    `check(q[1], value); \
    $put_array_vector("test.bits_da", 1, value); \
    `check(bits_da[1], bits); \
    $put_array_vector("test.bits_q", 1, value); \
    `check(bits_q[1], bits); \
    $put_array_vector("test.fixed_array", 0, value); \
    `check(fixed_array[0], value);

  initial begin
    da = new[3];
    bits_da = new[3];
    for (i = 0; i < 3; i = i + 1) begin
      da[i] = 65'd17;
      bits_da[i] = 65'd17;
      q.push_back(65'd17);
      bits_q.push_back(65'd17);
    end

    `write(65'b0, 65'b0);
    `write(65'h1ffffffffffffffff, 65'h1ffffffffffffffff);
    `write(65'h10000000180000001, 65'h10000000180000001);
    `write(65'bx, 65'b0);
    `write(65'bz, 65'b0);
    `write({1'bz, 1'bx, 30'd1, 1'bz, 1'bx, 31'd1}, 65'h00000000200000001);
    `write(65'd1, 65'd1);

    `check(da[0], 65'd17);
    `check(da[2], 65'd17);
    `check(q[0], 65'd17);
    `check(q[2], 65'd17);
    `check(bits_da[0], 65'd17);
    `check(bits_da[2], 65'd17);
    `check(bits_q[0], 65'd17);
    `check(bits_q[2], 65'd17);

    if (!failed) $display("PASSED");
  end

endmodule
