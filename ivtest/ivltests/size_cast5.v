module test();

function [31:0] cast_4uu(input [5:0] value);
  cast_4uu = 4'(value) + 'd0;
endfunction

function [31:0] cast_4us(input [5:0] value);
  cast_4us = 4'(value) + 'sd0;
endfunction

function [31:0] cast_4su(input signed [5:0] value);
  cast_4su = 4'(value) + 'd0;
endfunction

function [31:0] cast_4ss(input signed [5:0] value);
  cast_4ss= 4'(value) + 'sd0;
endfunction

function [31:0] cast_6uu(input [5:0] value);
  cast_6uu = 6'(value) + 'd0;
endfunction

function [31:0] cast_6us(input [5:0] value);
  cast_6us = 6'(value) + 'sd0;
endfunction

function [31:0] cast_6su(input signed [5:0] value);
  cast_6su = 6'(value) + 'd0;
endfunction

function [31:0] cast_6ss(input signed [5:0] value);
  cast_6ss= 6'(value) + 'sd0;
endfunction

function [31:0] cast_8uu(input [5:0] value);
  cast_8uu = 8'(value) + 'd0;
endfunction

function [31:0] cast_8us(input [5:0] value);
  cast_8us = 8'(value) + 'sd0;
endfunction

function [31:0] cast_8su(input signed [5:0] value);
  cast_8su = 8'(value) + 'd0;
endfunction

function [31:0] cast_8ss(input signed [5:0] value);
  cast_8ss= 8'(value) + 'sd0;
endfunction

localparam [31:0] result1a = cast_4uu(6'h3f);
localparam [31:0] result1b = cast_4us(6'h3f);
localparam [31:0] result1c = cast_4su(6'h3f);
localparam [31:0] result1d = cast_4ss(6'h3f);

localparam [31:0] result2a = cast_6uu(6'h3f);
localparam [31:0] result2b = cast_6us(6'h3f);
localparam [31:0] result2c = cast_6su(6'h3f);
localparam [31:0] result2d = cast_6ss(6'h3f);

localparam [31:0] result3a = cast_8uu(6'h3f);
localparam [31:0] result3b = cast_8us(6'h3f);
localparam [31:0] result3c = cast_8su(6'h3f);
localparam [31:0] result3d = cast_8ss(6'h3f);

reg failed = 0;

initial begin
  $display("%h", result1a);
  if (result1a !== 32'h0000000f) failed = 1;

  $display("%h", result1b);
  if (result1b !== 32'h0000000f) failed = 1;

  $display("%h", result1c);
  if (result1c !== 32'h0000000f) failed = 1;

  $display("%h", result1d);
  if (result1d !== 32'hffffffff) failed = 1;

  $display("%h", result2a);
  if (result2a !== 32'h0000003f) failed = 1;

  $display("%h", result2b);
  if (result2b !== 32'h0000003f) failed = 1;

  $display("%h", result2c);
  if (result2c !== 32'h0000003f) failed = 1;

  $display("%h", result2d);
  if (result2d !== 32'hffffffff) failed = 1;

  $display("%h", result3a);
  if (result3a !== 32'h0000003f) failed = 1;

  $display("%h", result3b);
  if (result3b !== 32'h0000003f) failed = 1;

  $display("%h", result3c);
  if (result3c !== 32'h000000ff) failed = 1;

  $display("%h", result3d);
  if (result3d !== 32'hffffffff) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule // main
