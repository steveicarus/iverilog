module top;
  time ioffset, ifuture;
  realtime offset, future;
  initial begin
    offset = 1.0;
    ioffset = 1;
    future = 20.0;
    ifuture = 120;
    #1;
    $display("----- Using real times -----");
    $display("The time one unit ago was        : %t", $realtime - 1.0);
    $display("The time one unit ago was        : %t", $realtime - offset);
    $display("The time now is                  : %t", $realtime);
    $display("One time unit from now it will be: %t", $realtime + 1.0);
    $display("The time at 20 will be           : %t", future);
    $display("The time at 40 will be           : %t", 40.0);
    #100;
    $display("\n----- Using integer times -----");
    $display("The time one unit ago was        : %t", $time - 1);
    $display("The time one unit ago was        : %t", $time - ioffset);
    $display("The time now is                  : %t", $time);
    $display("One time unit from now it will be: %t", $time + 1);
    $display("The time at 120 will be          : %t", ifuture);
    $display("The time at 140 will be          : %t", 140);
  end
endmodule
