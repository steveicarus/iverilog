// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module coupled(input c1, c2, sample, d, output reg q);
  wire delayed_clk, delayed_d;
  reg n1, n2;
  initial begin n1=0; n2=0; end
  always @(posedge sample) q <= delayed_d;
  specify
    $setuphold(posedge c1, posedge d, 131, -31, n1,,, delayed_clk, delayed_d);
    $setuphold(posedge c2, posedge d, 145, -47, n2);
  endspecify
endmodule
module tb;
  reg c1, c2, sample, d;
  wire q;
  coupled u(c1, c2, sample, d, q);
  initial begin
    c1=0; c2=0; sample=0; d=0;
    #1960 d=1;
    // Separate data delays would put the explicit wire at 1992 ps.
    // Sharing with the second check delays both consumers until 2008 ps.
    #40 sample=1;
    #7;
    if (q !== 0 || u.delayed_d !== 0) begin
      $display("FAILED: explicit and implicit consumers used different delays");
      $finish;
    end
    #2;
    if (u.delayed_d !== 1) begin
      $display("FAILED: delayed input was not delivered");
      $finish;
    end
    #191 c1=1;
    #100 c2=1;
    #1;
    if (u.n1 !== 0 || u.n2 !== 0) $display("FAILED: spurious timing violation");
    else $display("PASSED");
  end
endmodule
