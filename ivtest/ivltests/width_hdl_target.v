module tb;
  reg CLK=0;
  reg notifier=0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
