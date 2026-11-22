// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module cold_ff(input clk, input d, input rst_b, output reg q);
  wire dclk, dd, drst;
  reg rise_notify, fall_notify, reset_notify;
  initial begin rise_notify=0; fall_notify=0; reset_notify=0; end
  always @(posedge dclk or negedge drst)
    if (!drst) q <= 0;
    else q <= dd;
  specify
    $setuphold(posedge clk, posedge d, 0, 0, rise_notify,,, dclk, dd);
    $setuphold(posedge clk, negedge d, 0, 0, fall_notify,,, dclk, dd);
    $recrem(posedge rst_b, posedge clk, 0, 0, reset_notify,,, drst, dclk);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg clk, d, rst_b;
  wire q;
  cold_ff u(clk, d, rst_b, q);
  initial begin
    $sdf_annotate("ivltests/setuphold_sdf_cold_signed.sdf", u);
    #100 clk=0; d=0; rst_b=0;
    #200 rst_b=1;
    // Outside the raw window (1884,1938). Directly sampling d gives the wrong Q.
    #1680 d=1;
    #20 clk=1;
    #1;
    if (q !== 0 || u.dd !== 0 || {u.rise_notify, u.fall_notify, u.reset_notify} !== 3'b000) begin
      $display("FAILED: sampled D or reported a violation outside the raw window");
      $finish;
    end
    #150;
    if (u.dd !== 1) begin
      $display("FAILED: delayed data did not arrive");
      $finish;
    end
    clk=0;
    // Reference at 4000, raw transition at 3900 is inside (3884,3938).
    #1749 d=0;
    #100 clk=1;
    #1;
    if ({u.rise_notify, u.fall_notify, u.reset_notify} !== 3'b010) begin
      $display("FAILED: no notification inside the raw window");
      $finish;
    end
    clk=0; rst_b=0;
    #59 rst_b=1; #40 clk=1; #1;
    if ({u.rise_notify, u.fall_notify, u.reset_notify} !== 3'b011) begin
      $display("FAILED recovery did not use the shared unshifted clock");
      $finish;
    end
    $display("PASSED");
  end
endmodule
