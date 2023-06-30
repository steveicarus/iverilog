// Check that it is possible to declare module ports without specifing the
// direction.

bit failed = 1'b0;

`define check(val, exp) do begin \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): Expected `%d`, got `%d`.", `__LINE__, \
               (exp), (val)); \
      failed = 1'b1; \
    end \
  end while (0)

// All ports should be inout if no direction is specified
module M (wire [31:0] a, b, c);

  assign c = a ^ b;

  initial begin
    `check($bits(a), 32);
    `check($bits(b), 32);
    `check($bits(c), 32);
  end

endmodule

module test;

  wire [31:0] a, b, c;

  M i_m (a, b, c);

  `define A 'h01234567
  `define B 'hfedcba98

  assign a = `A;
  assign b = `B;

  initial begin
    #1

    `check(c, `A ^ `B);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
