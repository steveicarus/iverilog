timeunit 100ps / 10ps;

function [63:0] delay(input dummy);
  begin
    $printtimescale(top);
    $printtimescale;
    delay = 5ns;
  end
endfunction

module top();

timeunit 1ns / 1ps;

reg [63:0] t1;
reg [63:0] t2;

initial begin
  $printtimescale;
  t1 = delay(0);
  t2 = 5ns;
  $display("%0d %0d", t1, t2);
  if ((t1 === 50) && (t2 === 5))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
