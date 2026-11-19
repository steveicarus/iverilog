// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shared(C,D,R,E);
  input C,D,R,E;
  reg n; initial n=0;
  specify
    $setup(D &&& E, posedge C &&& R, 10, n);
    $hold(posedge C &&& R, D &&& E, 10, n);
  endspecify
endmodule
module omitted(C,D,R,E);
  input C,D,R,E;
  specify
    $setup(D &&& E, posedge C &&& R, 10);
    $hold(posedge C &&& R, D &&& E, 10);
  endspecify
endmodule
module highz(C,D,R,E);
  input C,D,R,E;
  reg n; initial n=1'bz;
  specify
    $setup(D &&& E, posedge C &&& R, 10, n);
    $hold(posedge C &&& R, D &&& E, 10, n);
  endspecify
endmodule

module tb;
  reg C,D,E;shared shared(C,D,E,E);omitted omitted(C,D,E,E);highz highz(C,D,E,E);
  initial begin C=0;D=0;E=0;#20 D=1;#5 C=1;#3 D=0;#2 C=0;
    #60 E=1;#10 D=1;#5 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sh_watch("tb.shared", "setup", "C", "posedge", "D", "any", 10.0, 10, "n", 1'b0, 1, 105);
    $sh_watch("tb.shared", "hold", "C", "posedge", "D", "any", 10.0, 10, "n", 1'b0, 1, 108);
    $sh_watch("tb.omitted", "setup", "C", "posedge", "D", "any", 10.0, 10, "", 1'b0, 1, 105);
    $sh_watch("tb.omitted", "hold", "C", "posedge", "D", "any", 10.0, 10, "", 1'b0, 1, 108);
    $sh_watch("tb.highz", "setup", "C", "posedge", "D", "any", 10.0, 10, "n", 1'bz, 1, 105);
    $sh_watch("tb.highz", "hold", "C", "posedge", "D", "any", 10.0, 10, "n", 1'bz, 1, 108);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
