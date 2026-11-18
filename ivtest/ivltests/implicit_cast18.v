// Check two-state conversion of part-selects in mixed blocking assignments.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic [3:0] four;
  bit [7:0] two;
  integer index = 4;

  initial begin
    two = 8'ha5;
    {four, two[3:0]} = 8'b0101xxxx;
    `check({four, two}, 12'h5a0);

    two = 8'ha5;
    {two[index +: 4], four} = 8'bzzzzxxxx;
    `check({four, two}, 12'bxxxx00000101);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
