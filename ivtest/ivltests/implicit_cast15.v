// Check that blocking assignments convert only two-state concatenation components.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic [3:0] four;
  bit [3:0] two, other;

  initial begin
    {four, two} = 8'bxxxx0101;
    `check({four, two}, 8'bxxxx0101);

    {two, four} = 8'b0101zzzz;
    `check({two, four}, 8'b0101zzzz);

    {four, two, other} = 12'bxxxx01011010;
    `check({four, two, other}, 12'bxxxx01011010);

    {two, four, other} = 12'b0101zzzz1010;
    `check({two, four, other}, 12'b0101zzzz1010);

    {two, other, four} = 12'b01011010xxxx;
    `check({two, other, four}, 12'b01011010xxxx);

    {two, other} = 8'bx1z0_z01x;
    `check({two, other}, 8'b0100_0010);

    {two, four} = 8'bx1z0_z01x;
    `check({two, four}, 8'b0100_z01x);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
