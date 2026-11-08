// Check that a negative $timeformat minimum width is treated as zero and does
// not overflow the buffer used to format %t.
`timescale 1ns/1ns
module top;
  reg [8*2048:1] result;

  initial begin
    $timeformat(-9, 2, " ns", -1000);
    #1 $sformat(result, "%t", $time);
    if (result === "1.00 ns") $display("PASSED");
    else $display("FAILED: got '%0s'", result);
    $finish(0);
  end
endmodule
