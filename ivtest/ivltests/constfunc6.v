// Test system function calls in constant functions
module constfunc6();

function [7:0] clog2(input [7:0] a);
  clog2 = $clog2(a);
endfunction

function real log10(input [7:0] a);
  log10 = $log10(a);
endfunction

function real sqrt(input real a);
  sqrt = $sqrt(a);
endfunction

function real pow_i(input [7:0] a, input [7:0] b);
  pow_i = $pow(a, b);
endfunction

function real pow_r(input real a, input real b);
  pow_r = $pow(a, b);
endfunction

localparam [7:0] clog2Result = clog2(25);

localparam real  log10Result = log10(100);

localparam real   sqrtResult =  sqrt(25.0);

localparam [7:0]  powIResult = pow_i(4, 3);

localparam real   powRResult = pow_r(4.0, 3.0);

reg failed;

initial begin
  failed = 0;
  $display("%0d", clog2Result);
  $display("%0g", log10Result);
  $display("%0g",  sqrtResult);
  $display("%0d",  powIResult);
  $display("%0g",  powRResult);
  if (clog2Result !== 8'd5)  failed = 1;
  if (log10Result !=  2.0)   failed = 1;
  if ( sqrtResult !=  5.0)   failed = 1;
  if ( powIResult !== 8'd64) failed = 1;
  if ( powRResult !=  64.0)  failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
