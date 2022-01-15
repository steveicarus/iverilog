module pr2890322;

reg [7:0] Array[0:1];

function [7:0] Sum;

input [7:0] a;
input [7:0] b;

begin
  Sum = a + b;
end

endfunction

wire [7:0] Result = Sum(Array[0], Array[1]);

initial begin
  Array[0] = 1;
  Array[1] = 2;
  #1 $display(Result);
  if (Result === 3)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
