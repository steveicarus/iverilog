// Check that an invalid array element does not skip other deassign targets.

module test;

  reg [3:0] a[1:0];
  reg [3:0] hi, lo;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    a[0] = 4'h6;
    a[1] = 4'h9;

    assign {hi, lo} = 8'hab;
    deassign {hi, a[2], lo};
    hi = 4'h1;
    lo = 4'h2;
    `check({hi, lo, a[0], a[1]}, 16'h1269);

    assign {hi, lo} = 8'h34;
    deassign {a[-1], hi, a[1'bx], lo, a[1'bz]};
    hi = 4'h5;
    lo = 4'h6;
    `check({hi, lo, a[0], a[1]}, 16'h5669);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
