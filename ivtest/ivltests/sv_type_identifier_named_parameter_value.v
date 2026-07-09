// Check that a named parameter override selector can match a visible typedef name.

module M #(parameter integer T = 0) (output [31:0] Y);
  assign Y = T;
endmodule

module test;

  typedef int T;

  reg failed;
  wire [31:0] y;

  M #(.T(32'd42)) i_m(.Y(y));

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    #1;
    `check(y, 32'd42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
