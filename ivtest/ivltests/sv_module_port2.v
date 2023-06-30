// Check that partial module ANSI port declarations are supported. Check that it
// is possible to redefine the unpacked dimensions only.

module test (input integer a, b[1:0], c[2:0][3:0]);

  bit failed = 1'b0;

`define check(val, exp) do begin \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): Expected `%d`, got `%d`.", `__LINE__, \
               (exp), (val)); \
      failed = 1'b1; \
    end \
  end while (0)

  initial begin
    `check($dimensions(a), 1);
    `check($dimensions(b), 2);
    `check($dimensions(c), 3);
    `check($bits(a), $bits(integer));
    `check($bits(b), $bits(integer) * 2);
    `check($bits(c), $bits(integer) * 3 * 4);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
