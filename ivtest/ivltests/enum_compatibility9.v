// Check full-width enum selects accept integral and other enum values.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef enum logic [1:0] { ZERO, ONE } enum_t;
  enum_t value;
  enum_t array[0:1];
  enum logic [4:5] { ASC_ZERO, ASC_ONE } ascending;
  enum logic [1:0] { OTHER = 3 } other;
  integer index;

  initial begin
    value[1:0] = 2;
    `check(value, 2'd2);
    value[0 +: 2] = 3;
    `check(value, 2'd3);
    value[1 -: 2] = 1;
    `check(value, 2'd1);
    other = OTHER;
    value[1:0] = other;
    `check(value, 2'd3);
    ascending[4:5] = 2;
    `check(ascending, 2'd2);
    ascending[4 +: 2] = 3;
    `check(ascending, 2'd3);
    ascending[5 -: 2] = 1;
    `check(ascending, 2'd1);
    index = 1;
    array[index][1:0] = 2;
    `check(array[1], 2'd2);
    value[1:0] <= 1;
    array[0][0 +: 2] <= 3;
    #1;
    `check(value, 2'd1);
    `check(array[0], 2'd3);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
