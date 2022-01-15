`timescale 1ns/1ps
module top;

  initial begin
    $timeformat(-9,6,"ns",20);
    $display("here");
    $display("in top, time: %t",$time);

    $finish(0);
  end

endmodule
