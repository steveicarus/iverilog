// Check that the signedness of methods on the built-in enum type is handled
// correctly when calling the method without parenthesis.

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

  enum shortint {
    A = -1,
    B = -2,
    C = -3
  } es;

  enum bit [15:0] {
    X = 65535,
    Y = 65534,
    Z = 65533
  } eu;

  initial begin
    es = B;
    eu = Y;

    // These all evaluate as signed
    `check($signed(eu.first) < 0)
    `check(es.first < 0)

    `check($signed(eu.last) < 0)
    `check(es.last < 0)

    `check($signed(eu.prev) < 0)
    `check(es.prev < 0)

    `check($signed(eu.next) < 0)
    `check(es.next < 0)

    // These all evaluate as unsigned
    `check(eu.first > 0)
    `check({es.first} > 0)
    `check($unsigned(es.first) > 0)
    `check(es.first > 16'h0)

    `check(eu.last > 0)
    `check({es.last} > 0)
    `check($unsigned(es.last) > 0)
    `check(es.last > 16'h0)

    `check(eu.prev > 0)
    `check({es.prev} > 0)
    `check($unsigned(es.prev) > 0)
    `check(es.prev > 16'h0)

    `check(eu.next > 0)
    `check({es.next} > 0)
    `check($unsigned(es.next) > 0)
    `check(es.next > 16'h0)

    // In arithmetic expressions if one operand is unsigned all operands are
    // considered unsigned
    z = eu.first + x;
    `check(z === 65545)
    z = eu.first + y;
    `check(z === 65545)

    z = eu.last + x;
    `check(z === 65543)
    z = eu.last + y;
    `check(z === 65543)

    z = eu.prev + x;
    `check(z === 65545)
    z = eu.prev + y;
    `check(z === 65545)

    z = eu.next + x;
    `check(z === 65543)
    z = eu.next + y;
    `check(z === 65543)

    z = es.first + x;
    `check(z === 65545)
    z = es.first + y;
    `check(z === 9)

    z = es.last + x;
    `check(z === 65543)
    z = es.last + y;
    `check(z === 7)

    z = es.prev + x;
    `check(z === 65545)
    z = es.prev + y;
    `check(z === 9)

    z = es.next + x;
    `check(z === 65543)
    z = es.next + y;
    `check(z === 7)

    // For ternary operators if one operand is unsigned the result is unsigend
    z = x ? eu.first : x;
    `check(z === 65535)
    z = x ? eu.first : y;
    `check(z === 65535)

    z = x ? eu.last : x;
    `check(z === 65533)
    z = x ? eu.last : y;
    `check(z === 65533)

    z = x ? eu.prev : x;
    `check(z === 65535)
    z = x ? eu.prev : y;
    `check(z === 65535)

    z = x ? eu.next : x;
    `check(z === 65533)
    z = x ? eu.next : y;
    `check(z === 65533)

    z = x ? es.first : x;
    `check(z === 65535)
    z = x ? es.first : y;
    `check(z === -1)

    z = x ? es.last : x;
    `check(z === 65533)
    z = x ? es.last : y;
    `check(z === -3)

    z = x ? es.prev : x;
    `check(z === 65535)
    z = x ? es.prev : y;
    `check(z === -1)

    z = x ? es.next : x;
    `check(z === 65533)
    z = x ? es.next : y;
    `check(z === -3)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
