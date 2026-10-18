// Check unsigned compound operations on enum selects and singleton concatenations.
// Exercise both run-time assignments and constant functions.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef enum logic signed [3:0] { NEGATIVE = -6 } enum_t;
  enum_t value;

  function [3:0] select_update(input [3:0] initial_value);
    enum_t local_value;
    local_value[3:0] = initial_value;
    local_value[3:0] /= 4'sd3;
    select_update = local_value;
  endfunction

  function [3:0] concat_update(input [3:0] initial_value);
    enum_t local_value;
    {local_value} = initial_value;
    {local_value} >>>= 1;
    concat_update = local_value;
  endfunction

  localparam [3:0] folded_select = select_update(4'ha);
  localparam [3:0] folded_concat = concat_update(4'ha);

  initial begin
    value = NEGATIVE;
    value[3:0] /= 4'sd3;
    `check(value, 4'd3);
    value = NEGATIVE;
    {value} >>>= 1;
    `check(value, 4'd5);
    `check(folded_select, 4'd3);
    `check(folded_concat, 4'd5);
    `check(select_update(4'ha), 4'd3);
    `check(concat_update(4'ha), 4'd5);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
