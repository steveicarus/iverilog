// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module implicit_ff(input clk, d);
  reg rise_notify, fall_notify;
  initial begin rise_notify = 0; fall_notify = 0; end
  specify
    $setuphold(posedge clk, posedge d, 0, 0, rise_notify);
    $setuphold(posedge clk, negedge d, 0, 0, fall_notify);
  endspecify
endmodule

module tb;
  reg clk, d0, d1;
  implicit_ff lo(clk, d0);
  implicit_ff hi(clk, d1);
  initial begin
    clk = 0; d0 = 0; d1 = 1;
    #0;
    $sdf_annotate("ivltests/setuphold_sdf_implicit_startup.sdf", lo);
    $sdf_annotate("ivltests/setuphold_sdf_implicit_startup.sdf", hi);
    // A zero HOLD approximation would wrongly fire at the first clock.
    #50 clk = 1; #1;
    if ({lo.rise_notify, lo.fall_notify, hi.rise_notify, hi.fall_notify} !== 4'b0000) begin
      $display("FAILED initialization was checked with zero HOLD");
      $finish;
    end
    clk = 0;
    // At 100 ps, the initial data transitions lie in the raw window (-16,38) ps.
    #49 clk = 1;
    #1;
    if ({lo.rise_notify, lo.fall_notify, hi.rise_notify, hi.fall_notify} !== 4'b0110)
      $display("FAILED wrong edge history for preinitialized implicit data");
    else
      $display("PASSED");
    $finish;
  end
endmodule
