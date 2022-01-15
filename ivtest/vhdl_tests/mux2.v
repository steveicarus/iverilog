/*
 * A simple test of some of the structural elements.
 */

module testbench;
  wire o;
  reg  i0, i1, sel;

  mux2 uut(o, i0, i1, sel);

  initial begin
    i0 <= 1;
    i1 <= 0;
    sel <= 0;
    #1;
    sel <= 1;
    #1;
    i1 <= 1;
    #1;
  end

  always @(o)
    $display(o);

endmodule // testbench

module mux2(c, a, b, s);
  input a, b, s;
  output c;
  wire s_bar, a_and_s_bar, b_and_s;

  not(s_bar, s);
  and(a_and_s_bar, a, s_bar);
  and(b_and_s, b, s);
  or(c, a_and_s_bar, b_and_s);

endmodule // mux2
