// Check that an index select of a packed array parameter of a typedef
// returns the whole element with the element's signedness.

module test;

  typedef logic signed [3:0] elem_t;

  localparam elem_t [1:0] P = 8'h3e;
  localparam elem_t [1:0][2:0] D = 24'h3e5a71;
  localparam elem_t [1:0] Q = 8'h03;
  localparam integer K = P[0];

  reg failed;
  integer i;
  reg [15:0] value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    i = 0;
    value = P[0];
    `check(value, 16'hfffe);
    value = P[i];
    `check(value, 16'hfffe);
    value = D[1];
    `check(value, 16'h03e5);
    `check(K, -2);
    value = D[0][0];
    `check(value, 16'h0001);
    value = Q[1:0];
    `check(value, 16'h0003);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
