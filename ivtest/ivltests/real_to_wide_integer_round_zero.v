// Check wide integer assignments of negative real fractions that round to zero.

module test;

  reg failed;
  real value;
  reg [128:0] unsigned_result;
  reg signed [128:0] signed_result;
  wire [128:0] unsigned_net = value;
  wire signed [128:0] signed_net = value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = -0.25;
    unsigned_result = value;
    signed_result = value;
    #1;
    `check(unsigned_result, 129'd0);
    `check(signed_result, 129'd0);
    `check(unsigned_net, 129'd0);
    `check(signed_net, 129'd0);

    value = -0.5;
    unsigned_result = value;
    signed_result = value;
    #1;
    `check(unsigned_result, {129{1'b1}});
    `check(signed_result, {129{1'b1}});
    `check(unsigned_net, {129{1'b1}});
    `check(signed_net, {129{1'b1}});

    value = 0.25;
    unsigned_result = value;
    signed_result = value;
    #1;
    `check(unsigned_result, 129'd0);
    `check(signed_result, 129'd0);
    `check(unsigned_net, 129'd0);
    `check(signed_net, 129'd0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
