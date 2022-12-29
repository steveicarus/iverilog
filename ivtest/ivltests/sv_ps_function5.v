// Check that the signedness of package scoped functions is handled correctly.

package P;
  function shortint s();
    return -1;
  endfunction

  function bit [15:0] u();
    return -1;
  endfunction
endpackage

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

  initial begin
    // These all evaluate as signed
    `check(P::s() < 0)
    `check($signed(P::u()) < 0)

    // These all evaluate as unsigned
    `check(P::u() > 0)
    `check({P::s()} > 0)
    `check($unsigned(P::s()) > 0)
    `check(P::s() > 16'h0)

    // In arithmetic expressions if one operand is unsigned all operands are
    // considered unsigned
    z = P::u() + x;
    `check(z === 65545)
    z = P::u() + y;
    `check(z === 65545)

    z = P::s() + x;
    `check(z === 65545)
    z = P::s() + y;
    `check(z === 9)

    // For ternary operators if one operand is unsigned the result is unsigend
    z = x ? P::u() : x;
    `check(z === 65535)
    z = x ? P::u() : y;
    `check(z === 65535)

    z = x ? P::s() : x;
    `check(z === 65535)
    z = x ? P::s() : y;
    `check(z === -1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
