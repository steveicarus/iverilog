// Check signed division and arithmetic right shift assignments to packed struct
// members, both at run time and in constant functions. Explicit part-selects of
// signed members must use unsigned division.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef struct packed {
    logic signed [3:0] s;
    logic [3:0] u;
  } packed_t;

  packed_t value;

  function [7:0] update(input [7:0] initial_value);
    packed_t local_value;
    local_value = initial_value;
    local_value.s /= 4'sd3;
    local_value.s >>>= 4'd1;
    update = local_value;
  endfunction

  localparam [7:0] folded = update(8'ha5);

  initial begin
    value = 8'ha5;
    value.s /= 4'sd3;
    `check(value, 8'he5);
    value.s >>>= 4'd1;
    `check(value, 8'hf5);
    `check(folded, 8'hf5);
    `check(update(8'ha5), 8'hf5);

    value = 8'ha5;
    value.s[3:0] /= 4'sd3;
    `check(value, 8'h35);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
