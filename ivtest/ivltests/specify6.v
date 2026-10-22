// Check that a specify path without an edge qualifier delays all transitions.

`timescale 1ns/1ns

module M(input value, output result);

  assign result = value;

  specify
    (value => (result : value)) = 5;
  endspecify

endmodule

module test;

  reg value;
  wire result;
  reg [3:0] values;
  integer from, to;
  reg failed;

  M m(value, result);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    values = 4'bzx10;

    for (from = 0; from < 4; from = from + 1) begin
      for (to = 0; to < 4; to = to + 1) begin
        if (from != to) begin
          value = values[from];
          #10;
          value = values[to];
          #1;
          `check(result, values[from]);
          #5;
          `check(result, values[to]);
        end
      end
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
