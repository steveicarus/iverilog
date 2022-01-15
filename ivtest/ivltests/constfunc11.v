// Test repeat statements inside a constant function
module constfunc11();

function [31:0] pow2(input [5:0] x);

begin:body
  pow2 = 1;
  repeat (x) begin
    pow2 = 2 * pow2;
  end
end

endfunction

localparam val0 = pow2(0);
localparam val1 = pow2(1);
localparam val2 = pow2(2);
localparam val3 = pow2(3);

reg failed;

initial begin
  failed = 0;
  $display("%0d", val0); if (val0 !== 1) failed = 1;
  $display("%0d", val1); if (val1 !== 2) failed = 1;
  $display("%0d", val2); if (val2 !== 4) failed = 1;
  $display("%0d", val3); if (val3 !== 8) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
