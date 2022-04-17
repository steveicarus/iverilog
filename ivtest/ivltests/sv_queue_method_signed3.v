// Check that the signedness of the element type of a queue is correctly handled
// whenn calling one of the pop methods with parenthesis.

module test;

  bit failed = 1'b0;

  `define check(x) \
    if (!(x)) begin \
      $display("FAILED(%0d): ", `__LINE__, `"x`"); \
      failed = 1'b1; \
    end

  int unsigned x = 10;
  int y = 10;
  int z;
  longint w;

  shortint qs[$];
  bit [15:0] qu[$];

  initial begin
    for (int i = 0; i < 16; i++) begin
      qu.push_back(-1);
      qs.push_back(-1);
    end

    // These all evaluate as signed
    `check($signed(qu.pop_back) < 0)
    `check(qs.pop_back < 0)

    `check($signed(qu.pop_front) < 0)
    `check(qs.pop_front < 0)

    // These all evaluate as unsigned
    `check(qu.pop_back > 0)
    `check({qs.pop_back} > 0)
    `check($unsigned(qs.pop_back) > 0)
    `check(qs.pop_back > 16'h0)

    `check(qu.pop_front > 0)
    `check({qs.pop_front} > 0)
    `check($unsigned(qs.pop_front) > 0)
    `check(qs.pop_front > 16'h0)

    // In arithmetic expressions if one operand is unsigned all operands are
    // considered unsigned
    z = qu.pop_back + x;
    `check(z === 65545)
    z = qu.pop_back + y;
    `check(z === 65545)

    z = qu.pop_front + x;
    `check(z === 65545)
    z = qu.pop_front + y;
    `check(z === 65545)

    z = qs.pop_back + x;
    `check(z === 65545)
    z = qs.pop_back + y;
    `check(z === 9)

    z = qs.pop_front + x;
    `check(z === 65545)
    z = qs.pop_front + y;
    `check(z === 9)

    // For ternary operators if one operand is unsigned the result is unsigend
    z = x ? qu.pop_back : x;
    `check(z === 65535)
    z = x ? qu.pop_back : y;
    `check(z === 65535)

    z = x ? qu.pop_front : x;
    `check(z === 65535)
    z = x ? qu.pop_front : y;
    `check(z === 65535)

    z = x ? qs.pop_back : x;
    `check(z === 65535)
    z = x ? qs.pop_back : y;
    `check(z === -1)

    z = x ? qs.pop_front : x;
    `check(z === 65535)
    z = x ? qs.pop_front : y;
    `check(z === -1)

    // Size return value is always positive, but check that it gets padded
    // properly
    w = x ? qu.size : 64'h123;
    `check(w === 64'h4)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
