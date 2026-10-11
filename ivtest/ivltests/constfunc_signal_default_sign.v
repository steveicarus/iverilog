// Check the signedness of default-initialized constant-function locals.

module test;

  function bit scalar_compare(input bit unused);
    bit signed [7:0] value;
    scalar_compare = value < -8'sd1;
  endfunction

  function [7:0] scalar_shift(input bit unused);
    reg signed [7:0] value;
    scalar_shift = value >>> 1;
  endfunction

  function bit array_compare(input integer index);
    bit signed [7:0] value [0:1];
    array_compare = value[index] < -8'sd1;
  endfunction

  function [7:0] array_shift(input integer index);
    reg signed [7:0] value [0:1];
    array_shift = value[index] >>> 1;
  endfunction

  function bit unsigned_compare(input bit unused);
    bit [7:0] value;
    unsigned_compare = value < -8'sd1;
  endfunction

  function [7:0] unsigned_shift(input bit unused);
    reg [7:0] value;
    unsigned_shift = value >>> 1;
  endfunction

  localparam SCALAR_COMPARE = scalar_compare(0);
  localparam [7:0] SCALAR_SHIFT = scalar_shift(0);
  localparam ARRAY_COMPARE = array_compare(1);
  localparam [7:0] ARRAY_SHIFT = array_shift(1);
  localparam UNSIGNED_COMPARE = unsigned_compare(0);
  localparam [7:0] UNSIGNED_SHIFT = unsigned_shift(0);

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(SCALAR_COMPARE, 1'b0);
    `check(SCALAR_SHIFT, 8'bxxxxxxxx);
    `check(ARRAY_COMPARE, 1'b0);
    `check(ARRAY_SHIFT, 8'bxxxxxxxx);
    `check(UNSIGNED_COMPARE, 1'b1);
    `check(UNSIGNED_SHIFT, 8'b0xxxxxxx);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
