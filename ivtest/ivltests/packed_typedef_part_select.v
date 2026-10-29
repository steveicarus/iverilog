// Check that a part select of a packed array of a signed typedef is
// unsigned and as wide as the elements it selects, while a select of one
// element keeps the sign of the typedef.

module test;

  typedef logic signed [3:0] elem_t;

  elem_t [1:0] v;
  elem_t [1:0][1:0] g;

  reg failed;
  reg [15:0] value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    v = 8'hee;
    g = 16'heeee;
    value = v[1:1];
    `check(value, 16'h000e);
    value = v[1 -: 1];
    `check(value, 16'h000e);
    value = g[1][1:1];
    `check(value, 16'h000e);
    value = v[1];
    `check(value, 16'hfffe);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
