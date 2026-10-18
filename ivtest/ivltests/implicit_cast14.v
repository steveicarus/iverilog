// Check two-state conversion through enum selects and singleton concatenations.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef enum bit [1:0] { ZERO, ONE } enum_t;
  enum_t value;
  enum bit [0:0] { BIT_ZERO, BIT_ONE } single_bit;
  enum logic [1:0] { FOUR_ZERO, FOUR_ONE } four;
  struct packed {
    logic [3:0] pad;
    enum_t value;
  } packed_value;

  initial begin
    value[1:0] = 2'b1x;
    `check(value, 2'b10);
    value[0 +: 2] = 2'bz1;
    `check(value, 2'b01);
    {value} = 2'b1z;
    `check(value, 2'b10);
    single_bit[0] = 1'bx;
    `check(single_bit, 1'b0);
    packed_value = 6'b1010_00;
    {packed_value.value} = 2'b1x;
    `check(packed_value, 6'b1010_10);
    four[1:0] = 2'b1x;
    `check(four, 2'b1x);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
