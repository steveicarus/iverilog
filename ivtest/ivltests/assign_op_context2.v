// Check that compound shift counts are self-determined.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic [3:0] count;
  logic [7:0] x;
  logic signed [7:0] s;
  logic [3:0] narrow;
  logic [7:0] large_count;

  initial begin
    count = 15;
    x = 1;
    x <<= (count + 4'd1);
    `check(x, 8'd1);
    x <<<= (4'd15 + 4'd1);
    `check(x, 8'd1);
    x = 128;
    x >>= (count + 4'd1);
    `check(x, 8'd128);
    s = -64;
    s >>>= (count + 4'd1);
    `check(s, -8'sd64);

    narrow = 1;
    narrow <<= 8'd16;
    `check(narrow, 4'd0);
    large_count = 16;
    narrow = 8;
    narrow >>= large_count;
    `check(narrow, 4'd0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
