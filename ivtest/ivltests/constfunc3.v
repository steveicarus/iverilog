// Test binary operators in constant functions
module constfunc3();

function [7:0] Add(input [7:0] l, input [7:0] r);
  Add = l + r;
endfunction

function [7:0] Mul(input [7:0] l, input [7:0] r);
  Mul = l * r;
endfunction

function [7:0] Div(input [7:0] l, input [7:0] r);
  Div = l / r;
endfunction

function [7:0] Pow(input [7:0] l, input [7:0] r);
  Pow = l ** r;
endfunction

function [7:0] And(input [7:0] l, input [7:0] r);
  And = l & r;
endfunction

function [7:0] Shift(input [7:0] l, input [7:0] r);
  Shift = l << r;
endfunction

function [7:0] Logic(input [7:0] l, input [7:0] r);
begin
  Logic[0] = l[0] && r[0];
  Logic[1] = l[1] && r[1];
  Logic[2] = l[2] && r[2];
  Logic[3] = l[3] && r[3];
  Logic[4] = l[4] || r[4];
  Logic[5] = l[5] || r[5];
  Logic[6] = l[6] || r[6];
  Logic[7] = l[7] || r[7];
end
endfunction

localparam [7:0] ResultAdd   = Add(8'h0f, 8'h0f);
localparam [7:0] ResultMul   = Mul(8'h0f, 8'h0f);
localparam [7:0] ResultDiv   = Div(8'hf0, 8'h0f);
localparam [7:0] ResultPow   = Pow(8'h02, 8'h05);
localparam [7:0] ResultAnd   = And(8'h0f, 8'h55);
localparam [7:0] ResultShift = Shift(8'h55, 8'h03);
localparam [7:0] ResultLogic = Logic(8'h33, 8'h55);

reg failed;

initial begin
  failed = 0;
  $display("%h", ResultAdd);
  $display("%h", ResultMul);
  $display("%h", ResultDiv);
  $display("%h", ResultPow);
  $display("%h", ResultAnd);
  $display("%h", ResultShift);
  $display("%h", ResultLogic);
  if (ResultAdd   !== 8'h1e) failed = 1;
  if (ResultMul   !== 8'he1) failed = 1;
  if (ResultDiv   !== 8'h10) failed = 1;
  if (ResultPow   !== 8'h20) failed = 1;
  if (ResultAnd   !== 8'h05) failed = 1;
  if (ResultShift !== 8'ha8) failed = 1;
  if (ResultLogic !== 8'h71) failed = 1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
