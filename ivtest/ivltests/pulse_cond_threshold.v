// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module pulse_threshold(CLK,EN);
  input CLK,EN;
  reg n=0;
  specify $width(posedge CLK &&& (EN === 1'b1),10,4,n); endspecify
  initial #1 $pulse_watch(n,4,10,3,65,89,155);
endmodule
module tb;
  reg CLK=0,EN=1;
  pulse_threshold dut(CLK,EN);
  initial begin
    #20 CLK=1; #3 CLK=0;
    #17 CLK=1; #4 CLK=0;
    #16 CLK=1; #5 CLK=0;
    #15 CLK=1; #9 CLK=0;
    #11 CLK=1; #10 CLK=0;
    #20 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
  end
  initial begin
    #120 EN=0; #20 EN=1;
    #30 $pulse_complete(1); $finish;
  end
endmodule
