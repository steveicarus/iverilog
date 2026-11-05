// Check that an invalid array element does not skip other force targets.

module test;

  wire [3:0] a[1:0];
  wire [3:0] hi, lo;
  reg failed;

  assign a[0] = 4'h6;
  assign a[1] = 4'h9;
  assign hi = 4'h0;
  assign lo = 4'h0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    #1;
    force {hi, a[2], lo} = 12'habc;
    #1;
    `check({hi, lo, a[0], a[1]}, 16'hac69);

    force {a[-1], hi, a[1'bx], lo, a[1'bz]} = 20'h12345;
    #1;
    `check({hi, lo, a[0], a[1]}, 16'h2469);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
