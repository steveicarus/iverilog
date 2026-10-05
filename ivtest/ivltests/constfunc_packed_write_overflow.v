// Check that constant functions do not truncate packed write indices.

module test;

  function [31:0] f(input [95:0] index);
    reg signed [95:0] signed_index;
    reg [7:0] ubit, upart, sbit, spart;
    begin
      signed_index = index;
      ubit = 8'hff;
      upart = 8'hff;
      sbit = 8'hff;
      spart = 8'hff;
      ubit[index] = 1'b0;
      upart[index +: 4] = 4'b0;
      sbit[signed_index] = 1'b0;
      spart[signed_index +: 4] = 4'b0;
      f = {ubit, upart, sbit, spart};
    end
  endfunction

  localparam [31:0] INSIDE = f(96'd0);
  localparam [31:0] PARTIAL_HIGH = f(96'd6);
  localparam [31:0] PARTIAL_LOW = f(-96'd2);
  localparam [31:0] HIGH32 = f(96'h80000001);
  localparam [31:0] HIGH64 = f(96'h8000000000000001);
  localparam [31:0] HIGH96 = f(96'h10000000000000001);
  localparam [31:0] LOW96 = f(-96'sh10000000000000002);

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(INSIDE, 32'hfef0_fef0);
    `check(PARTIAL_HIGH, 32'hbf3f_bf3f);
    `check(PARTIAL_LOW, 32'hffff_fffc);
    `check(HIGH32, 32'hffff_ffff);
    `check(HIGH64, 32'hffff_ffff);
    `check(HIGH96, 32'hffff_ffff);
    `check(LOW96, 32'hffff_ffff);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
