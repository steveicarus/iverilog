// Check variable initialisation in constant functions.
module constfunc8();

function real uninitialised_r(input dummy);
  real value;
  uninitialised_r = value;
endfunction

function [7:0] uninitialised_2(input dummy);
  reg bool [5:0] value;
  uninitialised_2 = {1'b1, value, 1'b1};
endfunction

function [7:0] uninitialised_4(input dummy);
  reg [5:0] value;
  uninitialised_4 = {1'b1, value, 1'b1};
endfunction

localparam result_r = uninitialised_r(0);
localparam result_2 = uninitialised_2(0);
localparam result_4 = uninitialised_4(0);

reg failed;

initial begin
  failed = 0;

  $display("%0g", result_r);
  if (result_r != 0.0) failed = 1;

  $display("%b", result_2);
  if (result_2 !== 8'b10000001) failed = 1;

  $display("%b", result_4);
  if (result_4 !== 8'b1xxxxxx1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
