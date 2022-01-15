// Test disable statements inside a constant function
module constfunc10();

function [31:0] pow2(input [5:0] x);

begin:body
  pow2 = 1;
  while (1) begin:loop
    if (x == 0) disable body;
    pow2 = 2 * pow2;
    x = x - 1;
    disable loop;
    pow2 = 0;
  end
end

endfunction

localparam val = pow2(8);

initial begin
  $display("%0d", val);
  if (val === 256)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
