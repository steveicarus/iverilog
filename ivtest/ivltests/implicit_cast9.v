// Test implicit casts during constant function input assignments.

module implicit_cast();

localparam real                  src_r  = -7;

localparam bit   unsigned  [7:0] src_u2 =  7;
localparam bit   signed    [7:0] src_s2 = -7;

localparam logic unsigned  [7:0] src_u4 =  7;
localparam logic signed    [7:0] src_s4 = -7;

localparam logic unsigned  [7:0] src_ux = 8'bx0z00111;
localparam logic signed    [7:0] src_sx = 8'bx0z00111;

function real cp_r(input real val);
  cp_r = val;
endfunction

function bit unsigned [3:0] cp_u2s(input bit unsigned [3:0] val);
  cp_u2s = val;
endfunction

function bit signed [3:0] cp_s2s(input bit signed [3:0] val);
  cp_s2s = val;
endfunction

function bit unsigned [11:0] cp_u2l(input bit unsigned [11:0] val);
  cp_u2l = val;
endfunction

function bit signed [11:0] cp_s2l(input bit signed [11:0] val);
  cp_s2l = val;
endfunction

function logic unsigned [3:0] cp_u4s(input logic unsigned [3:0] val);
  cp_u4s = val;
endfunction

function logic signed [3:0] cp_s4s(input logic signed [3:0] val);
  cp_s4s = val;
endfunction

function logic unsigned [11:0] cp_u4l(input logic unsigned [11:0] val);
  cp_u4l = val;
endfunction

function logic signed [11:0] cp_s4l(input logic signed [11:0] val);
  cp_s4l = val;
endfunction

localparam dst1_r = cp_r(src_r);
localparam dst2_r = cp_r(src_u4);
localparam dst3_r = cp_r(src_s4);
localparam dst4_r = cp_r(src_u2);
localparam dst5_r = cp_r(src_s2);
localparam dst6_r = cp_r(src_ux);
localparam dst7_r = cp_r(src_sx);

localparam dst1_u2s = cp_u2s(src_r);
localparam dst2_u2s = cp_u2s(src_u4);
localparam dst3_u2s = cp_u2s(src_s4);
localparam dst4_u2s = cp_u2s(src_u2);
localparam dst5_u2s = cp_u2s(src_s2);
localparam dst6_u2s = cp_u2s(src_ux);
localparam dst7_u2s = cp_u2s(src_sx);

localparam dst1_s2s = cp_s2s(src_r);
localparam dst2_s2s = cp_s2s(src_u4);
localparam dst3_s2s = cp_s2s(src_s4);
localparam dst4_s2s = cp_s2s(src_u2);
localparam dst5_s2s = cp_s2s(src_s2);
localparam dst6_s2s = cp_s2s(src_ux);
localparam dst7_s2s = cp_s2s(src_sx);

localparam dst1_u2l = cp_u2l(src_r);
localparam dst2_u2l = cp_u2l(src_u4);
localparam dst3_u2l = cp_u2l(src_s4);
localparam dst4_u2l = cp_u2l(src_u2);
localparam dst5_u2l = cp_u2l(src_s2);
localparam dst6_u2l = cp_u2l(src_ux);
localparam dst7_u2l = cp_u2l(src_sx);

localparam dst1_s2l = cp_s2l(src_r);
localparam dst2_s2l = cp_s2l(src_u4);
localparam dst3_s2l = cp_s2l(src_s4);
localparam dst4_s2l = cp_s2l(src_u2);
localparam dst5_s2l = cp_s2l(src_s2);
localparam dst6_s2l = cp_s2l(src_ux);
localparam dst7_s2l = cp_s2l(src_sx);

localparam dst1_u4s = cp_u4s(src_r);
localparam dst2_u4s = cp_u4s(src_u4);
localparam dst3_u4s = cp_u4s(src_s4);
localparam dst4_u4s = cp_u4s(src_u2);
localparam dst5_u4s = cp_u4s(src_s2);
localparam dst6_u4s = cp_u4s(src_ux);
localparam dst7_u4s = cp_u4s(src_sx);

localparam dst1_s4s = cp_s4s(src_r);
localparam dst2_s4s = cp_s4s(src_u4);
localparam dst3_s4s = cp_s4s(src_s4);
localparam dst4_s4s = cp_s4s(src_u2);
localparam dst5_s4s = cp_s4s(src_s2);
localparam dst6_s4s = cp_s4s(src_ux);
localparam dst7_s4s = cp_s4s(src_sx);

localparam dst1_u4l = cp_u4l(src_r);
localparam dst2_u4l = cp_u4l(src_u4);
localparam dst3_u4l = cp_u4l(src_s4);
localparam dst4_u4l = cp_u4l(src_u2);
localparam dst5_u4l = cp_u4l(src_s2);
localparam dst6_u4l = cp_u4l(src_ux);
localparam dst7_u4l = cp_u4l(src_sx);

localparam dst1_s4l = cp_s4l(src_r);
localparam dst2_s4l = cp_s4l(src_u4);
localparam dst3_s4l = cp_s4l(src_s4);
localparam dst4_s4l = cp_s4l(src_u2);
localparam dst5_s4l = cp_s4l(src_s2);
localparam dst6_s4l = cp_s4l(src_ux);
localparam dst7_s4l = cp_s4l(src_sx);

bit failed;

initial begin
  failed = 0;

  $display("cast to real");
  $display("%g", dst1_r); if (dst1_r != -7.0) failed = 1;
  $display("%g", dst2_r); if (dst2_r !=  7.0) failed = 1;
  $display("%g", dst3_r); if (dst3_r != -7.0) failed = 1;
  $display("%g", dst4_r); if (dst4_r !=  7.0) failed = 1;
  $display("%g", dst5_r); if (dst5_r != -7.0) failed = 1;
  $display("%g", dst6_r); if (dst6_r !=  7.0) failed = 1;
  $display("%g", dst7_r); if (dst7_r !=  7.0) failed = 1;

  $display("cast to small unsigned bit");
  $display("%d", dst1_u2s); if (dst1_u2s !== 4'd9) failed = 1;
  $display("%d", dst2_u2s); if (dst2_u2s !== 4'd7) failed = 1;
  $display("%d", dst3_u2s); if (dst3_u2s !== 4'd9) failed = 1;
  $display("%d", dst4_u2s); if (dst4_u2s !== 4'd7) failed = 1;
  $display("%d", dst5_u2s); if (dst5_u2s !== 4'd9) failed = 1;
  $display("%d", dst6_u2s); if (dst6_u2s !== 4'd7) failed = 1;
  $display("%d", dst7_u2s); if (dst7_u2s !== 4'd7) failed = 1;

  $display("cast to small signed bit");
  $display("%d", dst1_s2s); if (dst1_s2s !== -4'sd7) failed = 1;
  $display("%d", dst2_s2s); if (dst2_s2s !==  4'sd7) failed = 1;
  $display("%d", dst3_s2s); if (dst3_s2s !== -4'sd7) failed = 1;
  $display("%d", dst4_s2s); if (dst4_s2s !==  4'sd7) failed = 1;
  $display("%d", dst5_s2s); if (dst5_s2s !== -4'sd7) failed = 1;
  $display("%d", dst6_s2s); if (dst6_s2s !==  4'sd7) failed = 1;
  $display("%d", dst7_s2s); if (dst7_s2s !==  4'sd7) failed = 1;

  $display("cast to large unsigned bit");
  $display("%d", dst1_u2l); if (dst1_u2l !== 12'd4089) failed = 1;
  $display("%d", dst2_u2l); if (dst2_u2l !== 12'd7)    failed = 1;
  $display("%d", dst3_u2l); if (dst3_u2l !== 12'd4089) failed = 1;
  $display("%d", dst4_u2l); if (dst4_u2l !== 12'd7)    failed = 1;
  $display("%d", dst5_u2l); if (dst5_u2l !== 12'd4089) failed = 1;
  $display("%b", dst6_u2l); if (dst6_u2l !== 12'b000000000111) failed = 1;
  $display("%b", dst7_u2l); if (dst7_u2l !== 12'b000000000111) failed = 1;

  $display("cast to large signed bit");
  $display("%d", dst1_s2l); if (dst1_s2l !== -12'sd7) failed = 1;
  $display("%d", dst2_s2l); if (dst2_s2l !==  12'sd7) failed = 1;
  $display("%d", dst3_s2l); if (dst3_s2l !== -12'sd7) failed = 1;
  $display("%d", dst4_s2l); if (dst4_s2l !==  12'sd7) failed = 1;
  $display("%d", dst5_s2l); if (dst5_s2l !== -12'sd7) failed = 1;
  $display("%b", dst6_s2l); if (dst6_s2l !== 12'b000000000111) failed = 1;
  $display("%b", dst7_s2l); if (dst7_s2l !== 12'b000000000111) failed = 1;

  $display("cast to small unsigned logic");
  $display("%d", dst1_u4s); if (dst1_u4s !== 4'd9) failed = 1;
  $display("%d", dst2_u4s); if (dst2_u4s !== 4'd7) failed = 1;
  $display("%d", dst3_u4s); if (dst3_u4s !== 4'd9) failed = 1;
  $display("%d", dst4_u4s); if (dst4_u4s !== 4'd7) failed = 1;
  $display("%d", dst5_u4s); if (dst5_u4s !== 4'd9) failed = 1;
  $display("%d", dst6_u4s); if (dst6_u4s !== 4'd7) failed = 1;
  $display("%d", dst7_u4s); if (dst7_u4s !== 4'd7) failed = 1;

  $display("cast to small signed logic");
  $display("%d", dst1_s4s); if (dst1_s4s !== -4'sd7) failed = 1;
  $display("%d", dst2_s4s); if (dst2_s4s !==  4'sd7) failed = 1;
  $display("%d", dst3_s4s); if (dst3_s4s !== -4'sd7) failed = 1;
  $display("%d", dst4_s4s); if (dst4_s4s !==  4'sd7) failed = 1;
  $display("%d", dst5_s4s); if (dst5_s4s !== -4'sd7) failed = 1;
  $display("%d", dst6_s4s); if (dst6_s4s !==  4'sd7) failed = 1;
  $display("%d", dst7_s4s); if (dst7_s4s !==  4'sd7) failed = 1;

  $display("cast to large unsigned logic");
  $display("%d", dst1_u4l); if (dst1_u4l !== 12'd4089) failed = 1;
  $display("%d", dst2_u4l); if (dst2_u4l !== 12'd7)    failed = 1;
  $display("%d", dst3_u4l); if (dst3_u4l !== 12'd4089) failed = 1;
  $display("%d", dst4_u4l); if (dst4_u4l !== 12'd7)    failed = 1;
  $display("%d", dst5_u4l); if (dst5_u4l !== 12'd4089) failed = 1;
  $display("%b", dst6_u4l); if (dst6_u4l !== 12'b0000x0z00111) failed = 1;
  $display("%b", dst7_u4l); if (dst7_u4l !== 12'bxxxxx0z00111) failed = 1;

  $display("cast to large signed logic");
  $display("%d", dst1_s4l); if (dst1_s4l !== -12'sd7) failed = 1;
  $display("%d", dst2_s4l); if (dst2_s4l !==  12'sd7) failed = 1;
  $display("%d", dst3_s4l); if (dst3_s4l !== -12'sd7) failed = 1;
  $display("%d", dst4_s4l); if (dst4_s4l !==  12'sd7) failed = 1;
  $display("%d", dst5_s4l); if (dst5_s4l !== -12'sd7) failed = 1;
  $display("%b", dst6_s4l); if (dst6_s4l !==  12'b0000x0z00111) failed = 1;
  $display("%b", dst7_s4l); if (dst7_s4l !==  12'bxxxxx0z00111) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
