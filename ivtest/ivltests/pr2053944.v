module function_test();

function [3:0] Copy;

input [3:0] Value;

Copy = Value;

endfunction

reg [3:0] Value1;
reg [3:0] Value2;

event Start;

always @Start begin
  Value1 = Copy(1);
end

always @Start begin
  Value2 = Copy(2);
end

initial begin
  ->Start;
  #1 $display("%d %d", Value1, Value2);
end

endmodule
