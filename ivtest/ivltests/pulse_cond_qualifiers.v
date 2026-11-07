// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module pulse_qualifiers(CLK, EN);
  input CLK, EN;
  reg wb=0, w1=0, w0=0, pb=0, p1=0, p0=0;
  specify
    $width(posedge CLK &&& EN, 10, 0, wb);
    $width(posedge CLK &&& (EN === 1'b1), 10, 0, w1);
    $width(posedge CLK &&& (EN === 1'b0), 10, 0, w0);
    $period(posedge CLK &&& EN, 15, pb);
    $period(posedge CLK &&& (EN === 1'b1), 15, p1);
    $period(posedge CLK &&& (EN === 1'b0), 15, p0);
  endspecify
  initial begin
    #1;
    $pulse_watch(wb,4,10,4,45,55,145,155);
    $pulse_watch(w1,4,10,4,45,55,145,155);
    $pulse_watch(w0,4,10,3,25,115,125);
    $pulse_watch(pb,3,15,2,50,150);
    $pulse_watch(p1,3,15,2,50,150);
    $pulse_watch(p0,3,15,1,120);
  end
endmodule
module tb;
  reg CLK=0, EN=0;
  pulse_qualifiers dut(CLK,EN);
  initial begin
    #20 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #5 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #5 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #5 CLK=1; #5 CLK=0;
  end
  initial begin
    #30 EN=1; #30 EN=1'bx; #20 EN=1'bz;
    #20 EN=0; #30 EN=1;
    #40 $pulse_complete(6); $finish;
  end
endmodule
