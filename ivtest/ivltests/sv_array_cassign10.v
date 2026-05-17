// Check that continuous assignments to unpacked arrays preserve drive strength.

module test;

  reg failed;
  reg [8*3-1:0] s;

  wire driven[0:1];
  wire resolved[0:1];

  assign (weak1, weak0) driven = '{1'b1, 1'b0};
  assign resolved[0] = 1'b0;
  assign resolved[1] = 1'b1;
  assign (weak1, weak0) resolved = '{1'b1, 1'b0};

  `define check_str(val, exp) begin \
    $swrite(s, "%v", val); \
    if (s != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, s); \
      failed = 1'b1; \
    end \
  end

  initial begin
    failed = 1'b0;

    #0;
    `check_str(driven[0], "We1");
    `check_str(driven[1], "We0");
    `check_str(resolved[0], "St0");
    `check_str(resolved[1], "St1");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
