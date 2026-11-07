// Proposed qualified-history semantics; no declared notifier is required.
`timescale 1ps/1ps
module pulse_no_notifier(CLK, EN);
  input CLK, EN;
  specify
    $width(posedge CLK &&& EN, 10);
    $period(posedge CLK &&& (EN === 1'b1), 30);
  endspecify
  initial #1 $pulse_no_notifier_watch;
endmodule
module tb;
  reg CLK=0, EN=0;
  pulse_no_notifier dut(CLK, EN);
  initial begin
    #20 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #25 CLK=1; #5 CLK=0;
  end
  initial begin
    #10 EN=1; #40 EN=0; #20 EN=1;
    #40 $pulse_no_notifier_complete; $finish;
  end
endmodule
