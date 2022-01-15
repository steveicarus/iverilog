`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

// Test array variables inside a constant function
module constfunc14();

function [7:0] concat1(input [7:0] value);

reg [3:0] tmp[1:2];

begin
  {tmp[1], tmp[2]} = {value[3:0], value[7:4]};
  {concat1[3:0], concat1[7:4]} = {tmp[2], tmp[1]};
end

endfunction

function [7:0] concat2(input [7:0] value);

reg [3:0] tmp[1:2];

begin
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  {tmp[1], tmp[3]} = {value[3:0], value[7:4]};
  {concat2[3:0], concat2[7:4]} = {tmp[3], tmp[1]};
`else
  {tmp[1]} = {value[3:0]};
  {concat2[3:0], concat2[7:4]} = {4'bxxxx, tmp[1]};
`endif
end

endfunction

function [7:0] concat3(input [7:0] value);

reg [3:0] tmp[1:2];

begin
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  {tmp['bx], tmp[1]} = {value[3:0], value[7:4]};
  {concat3[3:0], concat3[7:4]} = {tmp['bx], tmp[1]};
`else
  {tmp[1]} = {value[7:4]};
  {concat3[3:0], concat3[7:4]} = {4'bxxxx, tmp[1]};
`endif
end

endfunction

localparam res1 = concat1(8'ha5);
localparam res2 = concat2(8'ha5);
localparam res3 = concat2(8'ha5);

reg failed = 0;

initial begin
  $display("%h", res1); if (res1 !== 8'h5a) failed = 1;
  $display("%h", res2); if (res2 !== 8'h5x) failed = 1;
  $display("%h", res3); if (res3 !== 8'h5x) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
