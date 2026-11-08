// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// All rejected entries would change limit 10 to 30 and add callbacks at 115/130.
// A valid following CELL must bind 20 and produce both episodes, proving parser continuation.
`timescale 1ps/1ps
module dut(D,C,E,F);
  input D,C,E,F;
  reg ns,nh; initial begin ns=0;nh=0;end
  specify
    $setup(D &&& E, posedge C &&& F === 1'b0, 10, ns);
    $hold(posedge C &&& F === 1'b0, D &&& E, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,E,F; dut u(D,C,E,F);
  dut valid(D,C,E,F);
  initial begin C=0;D=0;E=1;F=0;
    #100 D=1;#15 C=1;#15 D=0;#10 C=0;#60 D=1;#5 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_sdf_cond_reject.sdf",tb);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10, 10, "ns", 1, 205);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10, 10, "nh", 1, 208);
    $sh_watch("tb.valid", "setup", "C", "posedge", "D", "any", 20, 20, "ns", 2, 115, 205);
    $sh_watch("tb.valid", "hold", "C", "posedge", "D", "any", 20, 20, "nh", 2, 130, 208);
    $tchk_condition("tb.u", "ns", "data", "bare", "E", 0);
    $tchk_condition("tb.u", "ns", "reference", "case", "F", 0);
    $tchk_condition("tb.u", "nh", "data", "bare", "E", 0);
    $tchk_condition("tb.u", "nh", "reference", "case", "F", 0);
    $tchk_condition("tb.valid", "ns", "data", "bare", "E", 0);
    $tchk_condition("tb.valid", "ns", "reference", "case", "F", 0);
    $tchk_condition("tb.valid", "nh", "data", "bare", "E", 0);
    $tchk_condition("tb.valid", "nh", "reference", "case", "F", 0);
    $sh_start;
    #229; $sh_finish; $finish;
  end
endmodule
