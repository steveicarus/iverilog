// Check two-state compound assignment conversion for packed members and their selects.

module test;

  function [15:0] calc(input [7:0] divisor, input selected);
    struct packed {
      logic [7:0] four;
      bit [7:0] two;
    } value;

    value = 16'ha5c3;
    if (selected) begin
      value.four[7:4] = 4'bxz01;
      value.two[3:0] /= divisor;
      value.four[3:0] %= divisor;
    end else begin
      value.two /= divisor;
      value.four %= divisor;
    end
    calc = value;
  endfunction

  localparam [15:0] WHOLE_ZERO = calc(0, 0);
  localparam [15:0] WHOLE_NONZERO = calc(2, 0);
  localparam [15:0] SELECTED_ZERO = calc(0, 1);
  localparam [15:0] SELECTED_NONZERO = calc(2, 1);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(WHOLE_ZERO, 16'hxx00);
    `check(WHOLE_NONZERO, 16'h0161);
    `check(SELECTED_ZERO, 16'bxz01_xxxx_1100_0000);
    `check(SELECTED_NONZERO, 16'bxz01_0001_1100_0001);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
