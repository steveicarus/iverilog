// Check that the signedness of class properties are handled correctly when
// accessing the property on a class object.

module test;

  bit failed = 1'b0;

  `define check(x) \
    if (!(x)) begin \
      $display("FAILED(%0d): ", `__LINE__, `"x`"); \
      failed = 1'b1; \
    end

  class C;
    shortint s = -1;
    bit [15:0] u = -1;
  endclass

  C c;

  int unsigned x = 10;
  int y = 10;
  int z;

  initial begin
    c = new;

    // These all evaluate as signed
    `check(c.s < 0)
    `check($signed(c.u) < 0)

    // These all evaluate as unsigned
    `check(c.u > 0)
    `check({c.s} > 0)
    `check($unsigned(c.s) > 0)
    `check(c.s > 16'h0)

    // In arithmetic expressions if one operand is unsigned all operands are
    // considered unsigned
    z = c.u + x;
    `check(z === 65545)
    z = c.u + y;
    `check(z === 65545)

    z = c.s + x;
    `check(z === 65545)
    z = c.s + y;
    `check(z === 9)

    // For ternary operators if one operand is unsigned the result is unsigend
    z = x ? c.u : x;
    `check(z === 65535)
    z = x ? c.u : y;
    `check(z === 65535)

    z = x ? c.s : x;
    `check(z === 65535)
    z = x ? c.s : y;
    `check(z === -1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
