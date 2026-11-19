// X/Z disable bare and complemented controls. For a binary constant, case
// inequality yields true for X/Z, and logical comparisons yield X and qualify.
`timescale 1ps/1ps
module pulse_four_state(CLK, EN);
  input CLK, EN;
  reg wb, wn, wc0, wc1, wcn0, wcn1, we0, we1, wu0, wu1;
  reg pb, pn, pc0, pc1, pcn0, pcn1, pe0, pe1, pu0, pu1;
  specify
    $width(posedge CLK &&& EN, 10, 4, wb);
    $width(posedge CLK &&& ~EN, 10, 4, wn);
    $width(posedge CLK &&& (EN === 1'b0), 10, 4, wc0);
    $width(posedge CLK &&& (EN === 1'b1), 10, 4, wc1);
    $width(posedge CLK &&& (EN !== 1'b0), 10, 4, wcn0);
    $width(posedge CLK &&& (EN !== 1'b1), 10, 4, wcn1);
    $width(posedge CLK &&& (EN == 1'b0), 10, 4, we0);
    $width(posedge CLK &&& (EN == 1'b1), 10, 4, we1);
    $width(posedge CLK &&& (EN != 1'b0), 10, 4, wu0);
    $width(posedge CLK &&& (EN != 1'b1), 10, 4, wu1);
    $period(negedge CLK &&& EN, 15, pb);
    $period(negedge CLK &&& ~EN, 15, pn);
    $period(negedge CLK &&& (EN === 1'b0), 15, pc0);
    $period(negedge CLK &&& (EN === 1'b1), 15, pc1);
    $period(negedge CLK &&& (EN !== 1'b0), 15, pcn0);
    $period(negedge CLK &&& (EN !== 1'b1), 15, pcn1);
    $period(negedge CLK &&& (EN == 1'b0), 15, pe0);
    $period(negedge CLK &&& (EN == 1'b1), 15, pe1);
    $period(negedge CLK &&& (EN != 1'b0), 15, pu0);
    $period(negedge CLK &&& (EN != 1'b1), 15, pu1);
  endspecify
  initial begin
    {wb, wn, wc0, wc1, wcn0, wcn1, we0, we1, wu0, wu1} = 10'b0;
    {pb, pn, pc0, pc1, pcn0, pcn1, pe0, pe1, pu0, pu1} = 10'b0;
    #1;
    // Each watch checks callbacks and notifier transitions at these times.
    $pulse_watch(wb,  4,10,4,80,90,249,278);
    $pulse_watch(wn,  4,10,2,25,35);
    $pulse_watch(wc0, 4,10,2,25,35);
    $pulse_watch(wc1, 4,10,4,80,90,249,278);
    $pulse_watch(wcn0,4,10,8,80,90,135,145,190,200,249,278);
    $pulse_watch(wcn1,4,10,6,25,35,135,145,190,200);
    $pulse_watch(we0, 4,10,6,25,35,135,145,190,200);
    $pulse_watch(we1, 4,10,8,80,90,135,145,190,200,249,278);
    $pulse_watch(wu0, 4,10,8,80,90,135,145,190,200,249,278);
    $pulse_watch(wu1, 4,10,6,25,35,135,145,190,200);
    $pulse_watch(pb,  3,15,2,90,278);
    $pulse_watch(pn,  3,15,2,35,256);
    $pulse_watch(pc0, 3,15,2,35,256);
    $pulse_watch(pc1, 3,15,2,90,278);
    $pulse_watch(pcn0,3,15,4,90,145,200,278);
    $pulse_watch(pcn1,3,15,4,35,145,200,256);
    $pulse_watch(pe0, 3,15,4,35,145,200,256);
    $pulse_watch(pe1, 3,15,4,90,145,200,278);
    $pulse_watch(pu0, 3,15,4,90,145,200,278);
    $pulse_watch(pu1, 3,15,4,35,145,200,256);
  end
endmodule

module tb;
  reg CLK, EN;
  pulse_four_state dut(CLK, EN);

  task pulses;
    begin
      CLK=1; #5 CLK=0;
      #5 CLK=1; #5 CLK=0;
      #5 CLK=1; #10 CLK=0;
    end
  endtask

  initial begin
    EN=0;
    #66 EN=1;
    #54 EN=1'bx;
    #55 EN=1'bz;
    #55 EN=1;
    #12 EN=0;
    #6 EN=1;
    #6 EN=0;
    #4 EN=1;
  end
  initial begin
    CLK=0;
    // Repeat the same 5, 5, 10 ps widths for each EN state. The two
    // falling-to-falling intervals are 10 ps and exactly 15 ps.
    #20 pulses; // 20..50 ps, EN=0
    #25 pulses; // 75..105 ps, EN=1
    #25 pulses; // 130..160 ps, EN=X
    #25 pulses; // 185..215 ps, EN=Z

    // For EN=1, the width at 249 ps is 9 ps from the accepted opening
    // at 240 ps. The 246 ps opening is masked and would yield 3 ps,
    // below the threshold, if it overwrote the earlier opening.
    #25 CLK=1; // 240
    #4 CLK=0;  // 244 (masked for EN=1)
    #2 CLK=1;  // 246 (masked for EN=1)
    #3 CLK=0;  // 249
    #3 CLK=1;  // 252
    #4 CLK=0;  // 256 (masked for EN=1)
    #1 CLK=1;  // 257 (masked for EN=1)
    // The qualified falling-edge interval is 264-249 = 15 ps. A
    // spurious update at 256 ps would produce an 8 ps violation.
    #7 CLK=0;  // 264
    #8 CLK=1;  // 272
    #6 CLK=0;  // 278 (14 ps period, 6 ps width)
    #10 $pulse_complete(20);
    $finish;
  end
endmodule
