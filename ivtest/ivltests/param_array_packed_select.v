// Check that bit, part, and indexed part selects of a multi-dimension packed
// array parameter apply the last index to the dimension after the prefix
// indices, as they do for a variable.

module test;

  typedef logic signed [3:0] elem_t;

  localparam logic [1:0][3:0] M = 8'h3e;
  localparam logic [0:1][3:0] A = 8'h3e;
  localparam elem_t [1:0] S = 8'h3e;
  localparam elem_t [1:0][2:0] D = 24'h3e5a71;
  localparam signed SG = 8'sh3e;

  reg failed;
  integer i, j;
  reg [15:0] value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    i = 1;
    j = 2;
    value = M[1][2];
    `check(value, 16'h0000);
    value = M[0][4];
    `check(value, 16'b000000000000000x);
    value = M[1][j];
    `check(value, 16'h0000);
    value = M[1:1];
    `check(value, 16'h0003);
    value = M[1][2:1];
    `check(value, 16'h0001);
    value = M[1 -: 1];
    `check(value, 16'h0003);
    value = M[1][1 +: 2];
    `check(value, 16'h0001);
    value = M[i +: 1];
    `check(value, 16'h0003);
    j = 1;
    value = M[1][j +: 2];
    `check(value, 16'h0001);
    value = A[0 +: 1];
    `check(value, 16'h0003);
    value = S[0 +: 1];
    `check(value, 16'h000e);
    value = D[0][2];
    `check(value, 16'hfffa);
    value = SG[3:1];
    `check(value, 16'h0007);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
