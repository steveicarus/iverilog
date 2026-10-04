// Check that constant functions do not truncate packed select indices.

module test;

  function [19:0] f(input [95:0] index);
    bit [3:0] v2;
    logic [3:0] v4;
    reg signed [95:0] signed_index;
    v2 = 4'b1010;
    v4 = 4'b1010;
    signed_index = index;
    f = {v2[index], v2[index +: 4], v4[index], v4[index +: 4],
         v2[signed_index], v2[signed_index +: 4],
         v4[signed_index], v4[signed_index +: 4]};
  endfunction

  localparam [19:0] INSIDE = f(96'd0);
  localparam [19:0] PARTIAL_LOW = f(-96'd2);
  localparam [19:0] HIGH32 = f(96'h80000001);
  localparam [19:0] HIGH64 = f(96'h8000000000000001);
  localparam [19:0] HIGH96 = f(96'h10000000000000001);
  localparam [19:0] LOW96 = f(-96'sh10000000000000002);
  localparam [19:0] MAX32 = f(96'h7fffffff);
  localparam [19:0] MAX64 = f(96'h7fffffffffffffff);
  localparam [19:0] MIN64 = f(-96'sh8000000000000000);
  localparam [9:0] OOB = 10'b00000_xxxxx;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(INSIDE, 20'b01010_01010_01010_01010);
    `check(PARTIAL_LOW, {OOB, 10'b01000_x10xx});
    `check(HIGH32, {OOB, OOB});
    `check(HIGH64, {OOB, OOB});
    `check(HIGH96, {OOB, OOB});
    `check(LOW96, {OOB, OOB});
    `check(MAX32, {OOB, OOB});
    `check(MAX64, {OOB, OOB});
    `check(MIN64, {OOB, OOB});
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
