// Test concatenation operator in constant functions
module constfunc5();

function [23:0] Concat(input [3:0] a, input [3:0] b);
  Concat = {2{a, 4'hf, b}};
endfunction

localparam [23:0] Result1 = Concat(4'h5, 4'ha);
localparam [23:0] Result2 = Concat(4'ha, 4'h5);

reg failed;

initial begin
  failed = 0;
  $display("%h", Result1);
  $display("%h", Result2);
  if (Result1 !== 24'h5fa5fa) failed = 1;
  if (Result2 !== 24'haf5af5) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
