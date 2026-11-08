// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// The model uses 1ns/10ps; SDF uses 1ps. Limits 40/20ps are 0.04/0.02ns.
// Escaped event/qualifier names remain local identities; cold false conditions still bind.
`timescale 1ns/10ps
module dut(\event.pin ,\clock\pin ,\gate.pin ,\enable\pin );
  input \event.pin ,\clock\pin ,\gate.pin ,\enable\pin ;
  reg ns,nh; initial begin ns=0;nh=0;end
  specify
    $setup(\event.pin  &&& \gate.pin  === 1'b0, posedge \clock\pin  &&& \enable\pin , 0.01, ns);
    $hold(posedge \clock\pin  &&& \enable\pin , \event.pin  &&& \gate.pin  === 1'b0, 0.01, nh);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,A,E,F; dut u(A,C,E,F);
  initial begin C=0;A=0;E=1;F=0;#90 E=0;F=1;#10 A=1;#30 C=1;#15 A=0;end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_sdf_cond_escaped.sdf",tb);
    $sh_watch("tb.u", "setup", "clock\\pin", "posedge", "event.pin", "any", 0.04, 40, "ns", 1, 130);
    $sh_watch("tb.u", "hold", "clock\\pin", "posedge", "event.pin", "any", 0.02, 20, "nh", 1, 145);
    $tchk_condition("tb.u", "ns", "data", "case", "gate.pin", 0);
    $tchk_condition("tb.u", "ns", "reference", "bare", "enable\\pin", 0);
    $tchk_condition("tb.u", "nh", "data", "case", "gate.pin", 0);
    $tchk_condition("tb.u", "nh", "reference", "bare", "enable\\pin", 0);
    $sh_start;
    #229; $sh_finish; $finish;
  end
endmodule
