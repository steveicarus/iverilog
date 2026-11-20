`timescale 1ps/1ps
module pulse_cond_sdf_logical_not_leaf(input CLK, EN);
  specify
    $width(posedge CLK &&& ~EN, 10, 0);
    $period(posedge CLK &&& ~EN, 11);
  endspecify
endmodule

module tb;
  reg CLK = 0, EN = 0;
  pulse_cond_sdf_logical_not_leaf rejected(CLK, EN);
  pulse_cond_sdf_logical_not_leaf later(CLK, EN);
  initial begin
    #5;
    $sdf_annotate("ivltests/pulse_cond_sdf_logical_not.sdf", tb);
    $period_limit("tb.rejected", "width", "CLK", "posedge", 10.0, 10);
    $period_limit("tb.rejected", "period", "CLK", "posedge", 11.0, 11);
    $period_limit("tb.later", "width", "CLK", "posedge", 20.0, 20);
    $period_limit("tb.later", "period", "CLK", "posedge", 21.0, 21);
    $display("PASSED");
    $finish(0);
  end
endmodule
