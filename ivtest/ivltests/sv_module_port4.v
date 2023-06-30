// Check that partial module ANSI port declarations are supported. Check that it
// is possible to redefine the port kind only.



module test(
  output [3:0] a,
  var b, c,
  wire d);

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
    `check($bits(b), 1);
    `check($bits(c), 1);
    `check($bits(d), 1);

    // a and d are wires, b and c are variables
    `check(a, 4'bzzzz);
    `check(b, 1'bx);
    `check(c, 1'bx);
    `check(d, 1'bz);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
