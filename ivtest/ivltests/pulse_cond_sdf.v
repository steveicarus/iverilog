// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module pulse_sdf_leaf(CLK,EN);
  input CLK,EN;
  reg wu=0,w1=0,w0=0,pu=0,p1=0,p0=0;
  specify
    $width(posedge CLK,10,0,wu);
    $width(posedge CLK &&& (EN === 1'b1),10,0,w1);
    $width(posedge CLK &&& (EN === 1'b0),10,0,w0);
    $period(posedge CLK,20,pu);
    $period(posedge CLK &&& (EN === 1'b1),20,p1);
    $period(posedge CLK &&& (EN === 1'b0),20,p0);
  endspecify

endmodule
module tb;
  reg CLK=0,EN=0;
  pulse_sdf_leaf annotated(CLK,EN);
  pulse_sdf_leaf untouched(CLK,EN);
  initial begin
    #6;
    $pulse_watch(annotated.wu,4,30,4,115,145,205,230);
    $pulse_watch(annotated.w1,4,30,2,115,145);
    $pulse_watch(annotated.w0,4,30,2,205,230);
    $pulse_watch(annotated.pu,3,40,2,130,225);
    $pulse_watch(annotated.p1,3,40,1,130);
    $pulse_watch(annotated.p0,3,40,1,225);
    $pulse_watch(untouched.wu,4,10,2,205,230);
    $pulse_watch(untouched.w1,4,10,0);
    $pulse_watch(untouched.w0,4,10,2,205,230);
    $pulse_watch(untouched.pu,3,20,0);
    $pulse_watch(untouched.p1,3,20,0);
    $pulse_watch(untouched.p0,3,20,0);
  end
  initial begin
    #5 $sdf_annotate("ivltests/pulse_cond.sdf",annotated);
    #5 EN=1; #150 EN=0;
    #90 $pulse_complete(12); $finish;
  end
  initial begin
    #100 CLK=1; #15 CLK=0;
    #15 CLK=1; #15 CLK=0;
    #55 CLK=1; #5 CLK=0;
    #20 CLK=1; #5 CLK=0;
  end
endmodule
