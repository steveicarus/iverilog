// Check that module port buffers preserve drive strengths.

module M(input value, output result);
  assign (weak0, weak1) result = value;
endmodule

module N(input value);
endmodule

module test;

  reg data;
  wire value;
  reg [23:0] strength;
  reg failed;

  M i_m(data, value);
  N i_n(value);

  `define check(val, exp) \
    $sformat(strength, "%v", val); \
    if (strength !== exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, strength); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    data = 1'b0;
    #1;
    `check(i_m.result, "We0");
    `check(value, "We0");
    `check(i_n.value, "We0");

    data = 1'b1;
    #1;
    `check(i_m.result, "We1");
    `check(value, "We1");
    `check(i_n.value, "We1");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
