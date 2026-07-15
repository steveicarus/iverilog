// Check that a named port connection selector can match a visible typedef name.

module M(input [3:0] T, output [3:0] Y);
  assign Y = T;
endmodule

module test;

  typedef int T;

  reg failed;
  reg [3:0] value;
  wire [3:0] y;

  M i_m(.T(value), .Y(y));

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 4'ha;
    #1;
    `check(y, 4'ha);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
