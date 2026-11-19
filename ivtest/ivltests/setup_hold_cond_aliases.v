// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module same(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(posedge D &&& E, posedge C &&& E, 10, ns);
    $hold(posedge C &&& E, posedge D &&& E, 10, nh);
  endspecify
endmodule
module independent(C,D,R,E);
  input C,D,R,E;reg ns,nh;initial begin ns=0;nh=0;end
  specify
    $setup(posedge D &&& E, posedge C &&& R, 10, ns);
    $hold(posedge C &&& R, posedge D &&& E, 10, nh);
  endspecify
endmodule
module notifier_alias(C);
  input C;reg n;initial n=1;
  specify $hold(posedge C &&& n,posedge C &&& n,10,n);endspecify
endmodule
module aliasrole(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& C, posedge C &&& D, 10, ns);
    $hold(posedge C &&& D, D &&& C, 10, nh);
  endspecify
endmodule

module tb;
  reg P,G,C,D,J,R,E,N;
  same joined(P,P,1'b1,G);independent split(J,J,R,E);
  notifier_alias feedback(N);aliasrole roles(C,D,1'b1,1'b1);
  initial begin J=0;R=0;E=1;#20 J=1;#1 J=0;#2 R=1;E=0;#2 J=1;
    #1 J=0;#1 R=0;E=1;#1 J=1;#1 J=0;#6 R=1;E=1;#5 J=1;end
  initial begin N=0;#20 N=1;#1 N=0;#4 N=1;end
  initial begin P=0;G=0;#20 P=1;#1 P=0;#9 G=1;#10 P=1;#1 P=0;#9 G=0;#10 P=1;#1 P=0;#9 G=1;#10 P=1;end
  initial begin C=0;D=0;
    #20 D=1;#5 C=1;#3 D=0;#2 C=0;#10 D=1;#5 C=1;
    #3 D=0;#2 D=1;#5 C=0;#1 C=1;
  end
  initial begin
    #1;
    $sh_watch("tb.joined", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.joined", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1'b0, 2, 40, 80);
    $sh_watch("tb.roles", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 1, 56);
    $sh_watch("tb.roles", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 3, 28, 48, 50);
    $sh_watch("tb.split", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1'b0, 1, 25);
    $sh_watch("tb.split", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1'b0, 2, 28, 40);
    $sh_watch("tb.feedback", "hold", "C", "posedge", "C", "posedge", 10.0, 10, "n", 1'b1, 1, 20);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
