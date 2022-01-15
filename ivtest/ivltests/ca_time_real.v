`timescale 1ns/1ns
module top;
  real rtrig = 0.0;
  wire real rtm;

  assign rtm = rtrig * $realtime;

  initial begin
    $monitor(rtm);
    #1 rtrig = 1.0;
    #1 rtrig = 0.0;
    #1 rtrig = 1.0;
    #1 rtrig = 0.0;
  end

endmodule
