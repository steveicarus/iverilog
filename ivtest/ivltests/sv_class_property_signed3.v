// Check that the signedness of class properties are handled correctly when
// accessing the property in a class method.

module test;

  bit failed = 1'b0;

  `define check(x) \
    if (!(x)) begin \
      $display("FAILED: ", `"x`", `__LINE__); \
      failed = 1'b1; \
    end

  int unsigned x = 10;
  int y = 10;
  int z;

  class C;
    shortint s = -1;
    bit [15:0] u = -1;

    task test;

      // These all evaluate as signed
      `check(s < 0)
      `check($signed(u) < 0)

      // These all evaluate as unsigned
      `check(u > 0)
      `check({s} > 0)
      `check($unsigned(s) > 0)
      `check(s > 16'h0)

      // In arithmetic expressions if one operand is unsigned all operands are
      // considered unsigned
      z = u + x;
      `check(z === 65545)
      z = u + y;
      `check(z === 65545)

      z = s + x;
      `check(z === 65545)
      z = s + y;
      `check(z === 9)

      // For ternary operators if one operand is unsigned the result is unsigend
      z = x ? u : x;
      `check(z === 65535)
      z = x ? u : y;
      `check(z === 65535)

      z = x ? s : x;
      `check(z === 65535)
      z = x ? s : y;
      `check(z === -1)

      if (!failed) begin
        $display("PASSED");
      end
    endtask

  endclass

  C c;

  initial begin
    c = new;
    c.test();
  end

endmodule
