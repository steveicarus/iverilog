// Check mixed real/integral compound assignments in constant functions.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_real(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %f, got %f", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer multiply(input integer value, input real rhs);
    value *= rhs;
    multiply = value;
  endfunction

  function integer subtract(input integer value, input real rhs);
    value -= rhs;
    subtract = value;
  endfunction

  function [7:0] part(input logic [7:0] value, input real rhs);
    value[7:4] *= rhs;
    part = value;
  endfunction

  function [3:0] member(input real rhs);
    struct packed {
      logic signed [3:0] s;
      logic [3:0] u;
    } value;
    value.s = -3;
    value.s *= rhs;
    member = value[7:4];
  endfunction

  function [3:0] full_select(input logic signed [3:0] value, input real rhs);
    value[3:0] *= rhs;
    full_select = value;
  endfunction

  function real add(input real value, input logic signed [3:0] rhs);
    value += (rhs + 4'sd1);
    add = value;
  endfunction

  localparam integer product = multiply(3, 0.5);
  localparam integer negative = multiply(-3, 0.5);
  localparam integer difference = subtract(3, 0.5);
  localparam [7:0] selected = part(8'h35, 0.5);
  localparam [3:0] selected_member = member(0.5);
  localparam [3:0] unsigned_select = full_select(-3, 0.5);
  localparam real sum = add(0.5, 7);

  initial begin
    `check(product, 2);
    `check(negative, -2);
    `check(difference, 3);
    `check(selected, 8'h25);
    `check(selected_member, -4'sd2);
    `check(unsigned_select, 4'd7);
    `check_real(sum, -7.5);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
