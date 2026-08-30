module wide_multiply_carry;
  reg [255:0] a;
  reg [255:0] b;
  reg [511:0] product;
  integer failed;

  task check;
    input [511:0] actual;
    input [511:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED test %0d: got %h, expected %h",
                 test_id, actual, expected);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;

    a = 256'hffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
    b = a;
    product = a * b;
    check(product,
          512'hfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe0000000000000000000000000000000000000000000000000000000000000001,
          1);

    b = 256'hfedcba98765432100123456789abcdeff0e1d2c3b4a5968778695a4b3c2d1e0f;
    product = a * b;
    check(product,
          512'hfedcba98765432100123456789abcdeff0e1d2c3b4a5968778695a4b3c2d1e0e0123456789abcdeffedcba98765432100f1e2d3c4b5a69788796a5b4c3d2e1f1,
          2);

    a = 256'h243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c89;
    b = 256'h452821e638d01377be5466cf34e90c6cc0ac29b7c97c50dd3f84d5b5b5470917;
    product = a * b;
    check(product,
          512'h09cac66c371a78526f3beb8ae53010bbd74d0d4857f98974519b4f5e0436c828415b398c27354a34734b239152fed204e5f41eb09a3d585703e99a65f3db914f,
          3);

    b[193] = 1'bx;
    product = a * b;
    check(product, {512{1'bx}}, 4);

    b[193] = 1'bz;
    product = a * b;
    check(product, {512{1'bx}}, 5);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
