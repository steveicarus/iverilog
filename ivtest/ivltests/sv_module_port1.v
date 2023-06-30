// Check that partial module ANSI port declarations are supported. Check that it
// is possible to redefine the data type only.

module test (input [3:0] a, [1:0] b, integer c, wire d);

  bit failed = 1'b0;

`define check(val, exp) do begin \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): Expected `%d`, got `%d`.", `__LINE__, \
               (exp), (val)); \
      failed = 1'b1; \
    end \
  end while (0)

  initial begin
    `check($bits(a), 4);
    `check($bits(b), 2);
    `check($bits(c), $bits(integer));
    `check($bits(d), 1);

    // They should all be wires
    `check(a, 'z);
    `check(b, 'z);
    `check(c, 'z);
    `check(d, 'z);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
