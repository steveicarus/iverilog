// Check that statement labels can be used on ordinary statements.

module test;

  reg failed;
  integer i;
  integer value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 0;

    assign_label: value = value + 1;
    attr_label: (* keep = 1 *) value = value + 1;
    if_label: if (value == 2) value = value + 1;
    repeat_label: repeat (2) value = value + 1;
    for_label: for (i = 0; i < 2; i = i + 1) value = value + 1;

    `check(value, 7);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
