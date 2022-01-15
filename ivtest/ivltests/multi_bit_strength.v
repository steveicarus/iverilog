`timescale 1ns/1ps

module top;
  parameter length = 17;
  reg [length*8-1:0] result;
  wire [3:0] net;

  assign (pull1, strong0) net = 4'b0110;

  initial begin
    #1;
    $swrite(result, "%v", net);
    $display("All three lines should match:");
    $display("-----------------------------");
    $display("St0_Pu1_Pu1_St0 (reference)");
    $display("%v (display)\n%0s (swrite)", net, result);
  end
endmodule
