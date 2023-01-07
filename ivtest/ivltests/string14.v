// Check that the empty string "" is equivalent to 8'h00

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d): Expected '%0s', got '%0s'.", `__LINE__, exp, val); \
      failed = 1'b1; \
    end

  reg [47:0] s;
  reg [7:0] x;
  wire [7:0] y;

  assign y = "";

  initial begin
    `check("", 8'h00);
    `check($bits(""), 8);

    $sformat(s, ":%s:%0s:", "", "");
    `check(s, ": ::");

    x = 8'h00;
    $sformat(s, ":%s:%0s:", x, x);
    `check(s, ": ::");

    $sformat(s, ":%s:%0s:", y, y);
    `check(s, ": ::");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
