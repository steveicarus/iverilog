// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module pulse_alias(CLK,ALIAS);
  input CLK,ALIAS;
  // Two input ports share one actual net; this is not a continuous assignment.
  reg pd=0,pa=0,pn=0,wd=0,wn=0,wu=0;
  specify
    $period(posedge CLK &&& CLK,10,pd);
    $period(posedge CLK &&& ALIAS,10,pa);
    $period(negedge CLK &&& (CLK === 1'b0),10,pn);
    $width(posedge CLK &&& CLK,10,0,wd);
    $width(negedge CLK &&& (ALIAS === 1'b0),10,0,wn);
    $width(posedge CLK,10,0,wu);
  endspecify
  initial begin
    #1 $pulse_watch(pd,3,10,1,29);
    $pulse_watch(pa,3,10,1,29);
    $pulse_watch(pn,3,10,1,34);
    $pulse_watch(wd,4,10,0);
    $pulse_watch(wn,4,10,0);
    $pulse_watch(wu,4,10,3,25,34,49);
  end
endmodule
module tb;
  reg CLK=0;
  pulse_alias dut(CLK,CLK);
  initial begin
    #20 CLK=1; #5 CLK=0;
    #4 CLK=1; #5 CLK=0;
    #10 CLK=1; #5 CLK=0;
    #10 $pulse_complete(6); $finish;
  end
endmodule
