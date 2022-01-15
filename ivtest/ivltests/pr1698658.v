`timescale 1ns/10ps

module top;
  initial begin
    $timeformat(-6, 0, " uS", 8);
    #10 $display("The time is %t", $time);
    #1000 $display("The time is %t", $time);
  end
endmodule
