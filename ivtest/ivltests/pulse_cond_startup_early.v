// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
// The extra two time-zero edges create one declared PERIOD violation.
// A zero-duration WIDTH pulse remains at the zero threshold and is not counted.
`timescale 1ps/1ps
module pulse_startup_zero(CLK,EN);
  input CLK,EN;
  reg w=0,p=0;
  specify
    $width(posedge CLK &&& EN,10,0,w);
    $period(posedge CLK &&& EN,10,p);
  endspecify
  initial begin
    #1 $pulse_watch(w,4,10,2,5,13);
    $pulse_watch(p,3,10,2,0,8);
  end
endmodule
module pulse_startup_absent(CLK,EN);
  input CLK,EN;
  reg w=0,p=0;
  specify
    $width(posedge CLK &&& EN,10,0,w);
    $period(posedge CLK &&& EN,10,p);
  endspecify
  initial begin
    #1 $pulse_watch(w,4,10,2,35,44);
    $pulse_watch(p,3,10,1,39);
  end
endmodule
module tb;
  reg cz=0,ez=1,ca=0,ea=0;
  pulse_startup_zero zero(cz,ez);
  pulse_startup_absent absent(ca,ea);
  initial begin
    // The inactive-region transition is genuinely at time zero, after initial
    // values have propagated. It is not a fabricated zero timestamp sentinel.
    #0 cz=1; #0 cz=0; #0 cz=1;
    #5 cz=0; #3 cz=1; #5 cz=0;
    #10 cz=1; #10 cz=0;
  end
  initial begin
    #10 ca=1; #10 ca=0;
    #10 ca=1; #5 ca=0; #4 ca=1; #5 ca=0;
  end
  initial begin
    #15 ea=1;
    #35 $pulse_complete(4); $finish;
  end
endmodule
