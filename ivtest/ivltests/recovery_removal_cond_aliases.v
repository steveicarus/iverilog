// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module same(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(posedge R &&& RE, posedge C &&& RE, 10, ns);
    $removal(posedge R &&& RE,posedge C &&& RE, 10, nh);
  endspecify
endmodule
module independent(C,R,CE,RE);
  input C,R,CE,RE;reg ns,nh;initial begin ns=0;nh=0;end
  specify
    $recovery(posedge R &&& RE, posedge C &&& CE, 10, ns);
    $removal(posedge R &&& RE,posedge C &&& CE, 10, nh);
  endspecify
endmodule
module notifier_alias(C);
  input C;reg n;initial n=1;
  specify $recovery(posedge C &&& n,posedge C &&& n,10,n);endspecify
endmodule
module aliasrole(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& C, posedge C &&& R, 10, ns);
    $removal(R &&& C,posedge C &&& R, 10, nh);
  endspecify
endmodule

module tb;
  reg P,G,C,R,J,CE,RE,N;
  same joined(P,P,1'b1,G);independent split(J,J,CE,RE);
  notifier_alias feedback(N);aliasrole roles(C,R,1'b1,1'b1);
  initial begin J=0;CE=0;RE=1;#20 J=1;#1 J=0;#2 CE=1;RE=0;#2 J=1;
    #1 J=0;#1 CE=0;RE=1;#1 J=1;#1 J=0;#6 CE=1;RE=1;#5 J=1;end
  initial begin N=0;#20 N=1;#1 N=0;#4 N=1;end
  initial begin P=0;G=0;#20 P=1;#1 P=0;#9 G=1;#10 P=1;#1 P=0;#9 G=0;#10 P=1;#1 P=0;#9 G=1;#10 P=1;end
  initial begin C=0;R=0;
    #20 R=1;#5 C=1;#3 R=0;#2 C=0;#10 R=1;#5 C=1;
    #3 R=0;#2 R=1;#5 C=0;#1 C=1;
  end
  initial begin
    #1;

    $rr_watch("tb.joined", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 2, 40, 80);

    $rr_watch("tb.joined", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 0);

    $rr_watch("tb.roles", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 56);

    $rr_watch("tb.roles", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1'b0, 3, 28, 48, 50);

    $rr_watch("tb.split", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 2, 25, 40);

    $rr_watch("tb.split", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 28);

    $rr_watch("tb.feedback", "recovery", "C", "posedge", "C", "posedge", 10.0, 10, "n", 1'b1, 1, 20);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
