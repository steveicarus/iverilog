// Check that task output copies convert only two-state concatenation components.

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

  task copy(output [11:0] result, input [11:0] value);
    result = value;
  endtask

  initial begin
    copy({four, two, other}, 12'bxxxx01011010);
    `check({four, two, other}, 12'bxxxx01011010);

    copy({two, four, other}, 12'b0101zzzz1010);
    `check({two, four, other}, 12'b0101zzzz1010);

    copy({two, other, four}, 12'b01011010xxxx);
    `check({two, other, four}, 12'b01011010xxxx);

    copy({two, other, four}, 12'bx1z0_z01x_xz10);
    `check({two, other, four}, 12'b0100_0010_xz10);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
