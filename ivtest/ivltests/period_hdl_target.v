module tb;
  reg CLK=0;
  reg notifier=0;
  specify
    $period(posedge CLK, 10, notifier);
  endspecify
endmodule
