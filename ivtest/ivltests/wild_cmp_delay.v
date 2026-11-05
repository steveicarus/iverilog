// Check delayed wildcard comparisons with multi-bit operands.

module test;

  reg [7:0] a, b;
  integer delay;
  wire fixed_equal, fixed_unequal, variable_equal, variable_unequal;
  reg failed;

  assign #2 fixed_equal = a ==? b;
  assign #2 fixed_unequal = a !=? b;
  assign #(delay) variable_equal = a ==? b;
  assign #(delay) variable_unequal = a !=? b;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    delay = 4;
    a = 0;
    b = 0;
    #5;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b1010);

    a = 1;
    #1;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b1010);
    #2;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b0110);
    #2;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b0101);

    b = 8'b000000xz;
    #1;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b0101);
    #2;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b1001);
    #2;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'b1010);

    a = 8'bx0000000;
    b = 0;
    #5;
    `check({fixed_equal, fixed_unequal, variable_equal, variable_unequal}, 4'bxxxx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
