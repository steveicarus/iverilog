// Check that compound arithmetic uses the full operand width.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic [3:0] x;
  logic signed [3:0] s;
  logic [7:0] y;
  logic signed [7:0] t;
  logic [1:0] hi, lo;
  logic [7:0] word;
  logic [3:0] array[0:1];
  integer index;

  initial begin
    x = 12;
    x /= 8'd17;
    `check(x, 4'd0);
    x = 12;
    x %= 8'd17;
    `check(x, 4'd12);

    x = 12;
    x /= 65'h10000000000000001;
    `check(x, 4'd0);

    s = -6;
    t = 3;
    s /= t;
    `check(s, -4'sd2);
    s = -6;
    y = 3;
    s /= y;
    `check(s, 4'd3);
    x = 12;
    t = -2;
    x /= t;
    `check(x, 4'd0);

    y = 64;
    x = 2;
    y /= (x << 3'd3);
    `check(y, 8'd4);
    {hi, lo} = 12;
    {hi, lo} /= 8'd17;
    `check({hi, lo}, 4'd0);
    word = 8'hca;
    word[7:4] %= 8'd17;
    `check(word, 8'hca);
    array[1] = 12;
    index = 1;
    array[index] /= 8'd17;
    `check(array[1], 4'd0);

    x = 12;
    y = 8'bx0010001;
    x /= y;
    `check(x, 4'bxxxx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
