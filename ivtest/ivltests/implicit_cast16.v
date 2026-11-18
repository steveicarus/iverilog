// Check that nonblocking assignments convert only two-state concatenation components.

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
    {four, two} <= 8'bxxxx0101;
    #1;
    `check({four, two}, 8'bxxxx0101);

    {two, four} <= 8'b0101zzzz;
    #1;
    `check({two, four}, 8'b0101zzzz);

    {four, two, other} <= 12'bxxxx01011010;
    #1;
    `check({four, two, other}, 12'bxxxx01011010);

    {two, four, other} <= 12'b0101zzzz1010;
    #1;
    `check({two, four, other}, 12'b0101zzzz1010);

    {two, other, four} <= 12'b01011010xxxx;
    #1;
    `check({two, other, four}, 12'b01011010xxxx);

    {two, four} <= 8'bx1z0_z01x;
    #1;
    `check({two, four}, 8'b0100_z01x);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
