// Check the compiler fails gracefully
module test;

function [w-1:0] copy;

input [w-1:0] z;

begin
  copy = z;
end

endfunction

endmodule
