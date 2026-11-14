// Check that concatenated ports connect members with different directions.

module M(.p({a, b}));

  input [1:0] a;
  output [1:0] b;

  assign b = ~a;

endmodule

module N(.p({b, a}));

  input [1:0] a;
  output [1:0] b;

  assign b = ~a;

endmodule

module test;

  reg [1:0] value;
  wire [1:0] a = value;
  wire [1:0] b, c;
  reg failed;
  integer i;

  M i_m(.p({a, b}));
  N i_n(.p({c, a}));

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    for (i = 0; i < 4; i = i + 1) begin
      value = i;
      #1;
      `check(i_m.a, value);
      `check(i_n.a, value);
      `check(b, ~value);
      `check(c, ~value);
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
