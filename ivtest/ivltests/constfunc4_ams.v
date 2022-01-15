// Test unary operators in constant functions
module constfunc4();

function [7:0] LAbs(input signed [7:0] x);
  LAbs = abs(x);
endfunction

function real RAbs(input real x);
  RAbs = abs(x);
endfunction

localparam [7:0] ResultLAb1 = LAbs(8'sh01);
localparam [7:0] ResultLAb2 = LAbs(8'shff);
localparam real  ResultRAb1 = RAbs( 2.0);
localparam real  ResultRAb2 = RAbs(-2.0);

reg failed;

initial begin
  failed = 0;
  $display("%h", ResultLAb1);
  $display("%h", ResultLAb2);
  $display("%g", ResultRAb1);
  $display("%g", ResultRAb2);
  if (ResultLAb1 !== 8'h01) failed = 1;
  if (ResultLAb2 !== 8'h01) failed = 1;
  if (ResultRAb1 !=  2.0)   failed = 1;
  if (ResultRAb2 !=  2.0)   failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
