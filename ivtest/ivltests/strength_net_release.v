// Check whole and partial release of a strength-aware net.

module test;

  reg [1:0] source;
  wire [1:0] value;
  wire [1:0] observed = ~value;
  reg failed;

  bufif1 (weak0, weak1) drive[1:0] (value, source, 1'b1);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    source = 2'b01;
    #1;
    `check(observed, 2'b10);

    force value = 2'b11;
    #1;
    `check(observed, 2'b00);

    source = 2'b10;
    #1;
    release value;
    #1;
    `check(observed, 2'b01);

    force value[0] = 1'b1;
    #1;
    `check(observed, 2'b00);

    release value[0];
    #1;
    `check(observed, 2'b01);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
