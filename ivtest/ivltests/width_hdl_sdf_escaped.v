// SDF names are decoded once; escaped punctuation remains a local scalar.
`timescale 1ps/1ps
module width_dot(input wire \CLK.dot );
  reg notifier=0;
  specify
    $width(posedge \CLK.dot , 10, 0, notifier);
  endspecify
endmodule
module width_backslash(input wire \CLK\tail );
  reg notifier=0;
  specify
    $width(posedge \CLK\tail , 10, 0, notifier);
  endspecify
endmodule
module width_nested(input wire CLK);
endmodule
module width_hierarchy(input wire CLK);
  reg notifier=0;
  width_nested nested(CLK);
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module tb;
  reg CLK=0;
  width_dot dotted(CLK);
  width_backslash slashed(CLK);
  width_hierarchy hierarchy(CLK);
  width_history_check #(2,115,209) check0(dotted.notifier);
  width_history_check #(2,115,209) check1(slashed.notifier);
  width_history_check #(1,209,0) check2(hierarchy.notifier);
  initial begin
    #5;
    $sdf_annotate("ivltests/width_hdl_sdf_escaped.sdf", tb);
    $width_escaped_limits;
    #95 CLK=1;
    #15 CLK=0;
    #85 CLK=1;
    #9 CLK=0;
    #91 CLK=1;
    #20 CLK=0;
    #10;
    check0.complete;
    check1.complete;
    check2.complete;
    $display("PASSED");
    $finish;
  end
endmodule
`include "width_history_check.vh"
