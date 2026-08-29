// Check that enum named constants only affect following references.

typedef enum logic [1:0] {
  VALUE = 1
} outer_t;

module test;

  localparam value_before = VALUE;

  typedef enum logic [1:0] {
    VALUE = 2
  } inner_t;

  localparam value_after = VALUE;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(value_before, 2'd1);
    `check(value_after, 2'd2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
