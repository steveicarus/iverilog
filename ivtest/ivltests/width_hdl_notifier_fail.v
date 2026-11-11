`timescale 1ps/1ps
module tb;
  wire CLK, notifier;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
