// Check singleton concatenations do not enforce enum assignment restrictions.

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
  struct packed {
    logic [3:0] pad;
    enum_t value;
  } packed_value;
  reg [1:0] four;

  initial begin
    {value} = 2;
    `check(value, 2'd2);
    {{value}} = 3;
    `check(value, 2'd3);
    {array[1]} = 1;
    `check(array[1], 2'd1);
    packed_value = 6'b1010_00;
    {packed_value.value} = 3;
    `check(packed_value, 6'b1010_11);
    {value, four} = 4'b10xz;
    `check(value, 2'd2);
    `check(four, 2'bxz);
    {value} <= 1;
    #1;
    `check(value, 2'd1);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
