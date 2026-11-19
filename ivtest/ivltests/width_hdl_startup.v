// The t=0 zero-width pulse is not a violation: 0 < width is false.
// The 5 ps pulse closes at 15 ps and violates the 10 ps limit.
`timescale 1ps/1ps
module startup_width(CLK, notifier);
  input CLK;
  output notifier;
  reg notifier;
  initial notifier = 0;
  specify $width(posedge CLK, 10, 0, notifier); endspecify
endmodule
module tb;
  reg CLK;
  wire observed;
  startup_width dut(CLK, observed);
  width_history_check #(1, 15, 0) history(observed);
  initial begin
    CLK=0; #0 CLK=1; #0 CLK=0;
    #10 CLK=1; #5 CLK=0;
    #5 history.complete;
    $display("PASSED"); $finish;
  end
endmodule
`include "width_history_check.vh"
