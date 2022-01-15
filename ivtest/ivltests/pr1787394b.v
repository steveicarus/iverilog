/*
 * This should produce x StL x StL
 *
 * When using both nmos and pmos you get the correct
 * result if only one of the control signals is X, but
 * when used individually you get StX not StL. This
 * appears to be a vec4 vs vec8 problem. R versions
 * give similar results.
 *
 * If both control inputs are X and the input is 0 the
 * value is calculated incorrectly.
 */

module top;
  reg nctl, pctl, b;
  wire c, d;
  initial begin
    $display("These should all produce:\nx StL x StL\n-----------");
    $monitor("c=%b(%v), d=%b(%v), b=%b, nctl=%b, pctl=%b", c, c, d, d, b, nctl, pctl);
    b = 0;
    nctl = 0;
    pctl = 1'bx;
    #1 nctl = 1'bx;
    #1 b = 1;
  end

  //nmos n1 (c, b, nctl);
  //pmos p1 (c, b, pctl);
  //pmos p2 (d, b, pctl);
  bufif1 n1 (c, b, nctl);
  bufif0 p1 (c, b, pctl);
  bufif0 p2 (d, b, pctl);
endmodule
