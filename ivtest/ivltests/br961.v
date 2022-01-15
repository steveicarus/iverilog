module test;

localparam w = 8;

function [w-1:0] copy;

input [w-1:0] w;

begin
  copy = w;
end

endfunction

reg [w-1:0] value;

initial begin
  value = copy(21);
  $display("%d", value);
  if (value === 21)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
