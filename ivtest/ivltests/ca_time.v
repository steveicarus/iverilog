`timescale 1ns/1ns
module top;
  reg itrig = 1'b0;
  wire [31:0] tm, stm;

  assign tm = itrig * $time;
  assign stm = itrig * $stime;

  initial begin
    $monitor(tm,, stm);
    #1 itrig = 1'b1;
    #1 itrig = 1'b0;
    #1 itrig = 1'b1;
    #1 itrig = 1'b0;
  end

endmodule
