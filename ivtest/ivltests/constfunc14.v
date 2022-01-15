// Test concatenation inside a constant function
module constfunc14();

function [7:0] concat1(input [7:0] value);

reg [3:0] tmp1;
reg [3:0] tmp2;

begin
  {tmp1, tmp2} = {value[3:0], value[7:4]};
  {concat1[3:0], concat1[7:4]} = {tmp2, tmp1};
end

endfunction

function [7:0] concat2(input [7:0] value);

reg [2:0] tmp1;
reg [3:0] tmp2;

begin
  {tmp1, tmp2} = {value[3:0], value[7:4]};
  {concat2[3:0], concat2[7:4]} = {tmp2, tmp1};
end

endfunction

function [7:0] concat3(input [7:0] value);

reg signed [2:0] tmp1;
reg signed [2:0] tmp2;

begin
  {tmp1, tmp2} = {value[2:0], value[6:4]};
  concat3[7:4] = tmp1;
  concat3[3:0] = tmp2;
end

endfunction

localparam res1 = concat1(8'h5a);
localparam res2 = concat2(8'h5a);
localparam res3 = concat3(8'h5a);

reg failed = 0;

initial begin
  $display("%h", res1); if (res1 !== 8'ha5) failed = 1;
  $display("%h", res2); if (res2 !== 8'ha2) failed = 1;
  $display("%h", res3); if (res3 !== 8'h2d) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
