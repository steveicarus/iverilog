// Test implicit casts during module input assignments.

`ifdef __ICARUS__
  `define SUPPORT_REAL_NETS_IN_IVTEST
  `define SUPPORT_TWO_STATE_NETS_IN_IVTEST
`endif

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
module cp_r(output wire real dst,
            input  wire real src);
  assign dst = src;
endmodule
`endif

`ifdef SUPPORT_TWO_STATE_NETS_IN_IVTEST
module cp_u2s(output wire bit unsigned [3:0] dst,
              input  wire bit unsigned [3:0] src);
  assign dst = src;
endmodule

module cp_s2s(output wire bit signed [3:0] dst,
              input  wire bit signed [3:0] src);
  assign dst = src;
endmodule

module cp_u2l(output wire bit unsigned [11:0] dst,
              input  wire bit unsigned [11:0] src);
  assign dst = src;
endmodule

module cp_s2l(output wire bit signed [11:0] dst,
              input  wire bit signed [11:0] src);
  assign dst = src;
endmodule
`endif

module cp_u4s(output wire logic unsigned [3:0] dst,
              input  wire logic unsigned [3:0] src);
  assign dst = src;
endmodule

module cp_s4s(output wire logic signed [3:0] dst,
              input  wire logic signed [3:0] src);
  assign dst = src;
endmodule

module cp_u4l(output wire logic unsigned [11:0] dst,
              input  wire logic unsigned [11:0] src);
  assign dst = src;
endmodule

module cp_s4l(output wire logic signed [11:0] dst,
              input  wire logic signed [11:0] src);
  assign dst = src;
endmodule

module implicit_cast();

real                  src_r;

bit   unsigned  [7:0] src_u2;
bit   signed    [7:0] src_s2;

logic unsigned  [7:0] src_u4;
logic signed    [7:0] src_s4;

logic unsigned  [7:0] src_ux;
logic signed    [7:0] src_sx;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
wire real                  dst1_r;
wire real                  dst2_r;
wire real                  dst3_r;
wire real                  dst4_r;
wire real                  dst5_r;
wire real                  dst6_r;
wire real                  dst7_r;
`endif

`ifdef SUPPORT_TWO_STATE_NETS_IN_IVTEST
wire bit   unsigned  [3:0] dst1_u2s;
wire bit   unsigned  [3:0] dst2_u2s;
wire bit   unsigned  [3:0] dst3_u2s;
wire bit   unsigned  [3:0] dst4_u2s;
wire bit   unsigned  [3:0] dst5_u2s;
wire bit   unsigned  [3:0] dst6_u2s;
wire bit   unsigned  [3:0] dst7_u2s;

wire bit   signed    [3:0] dst1_s2s;
wire bit   signed    [3:0] dst2_s2s;
wire bit   signed    [3:0] dst3_s2s;
wire bit   signed    [3:0] dst4_s2s;
wire bit   signed    [3:0] dst5_s2s;
wire bit   signed    [3:0] dst6_s2s;
wire bit   signed    [3:0] dst7_s2s;

wire bit   unsigned [11:0] dst1_u2l;
wire bit   unsigned [11:0] dst2_u2l;
wire bit   unsigned [11:0] dst3_u2l;
wire bit   unsigned [11:0] dst4_u2l;
wire bit   unsigned [11:0] dst5_u2l;
wire bit   unsigned [11:0] dst6_u2l;
wire bit   unsigned [11:0] dst7_u2l;

wire bit   signed   [11:0] dst1_s2l;
wire bit   signed   [11:0] dst2_s2l;
wire bit   signed   [11:0] dst3_s2l;
wire bit   signed   [11:0] dst4_s2l;
wire bit   signed   [11:0] dst5_s2l;
wire bit   signed   [11:0] dst6_s2l;
wire bit   signed   [11:0] dst7_s2l;
`endif

wire logic unsigned  [3:0] dst1_u4s;
wire logic unsigned  [3:0] dst2_u4s;
wire logic unsigned  [3:0] dst3_u4s;
wire logic unsigned  [3:0] dst4_u4s;
wire logic unsigned  [3:0] dst5_u4s;
wire logic unsigned  [3:0] dst6_u4s;
wire logic unsigned  [3:0] dst7_u4s;

wire logic signed    [3:0] dst1_s4s;
wire logic signed    [3:0] dst2_s4s;
wire logic signed    [3:0] dst3_s4s;
wire logic signed    [3:0] dst4_s4s;
wire logic signed    [3:0] dst5_s4s;
wire logic signed    [3:0] dst6_s4s;
wire logic signed    [3:0] dst7_s4s;

wire logic unsigned [11:0] dst1_u4l;
wire logic unsigned [11:0] dst2_u4l;
wire logic unsigned [11:0] dst3_u4l;
wire logic unsigned [11:0] dst4_u4l;
wire logic unsigned [11:0] dst5_u4l;
wire logic unsigned [11:0] dst6_u4l;
wire logic unsigned [11:0] dst7_u4l;

wire logic signed   [11:0] dst1_s4l;
wire logic signed   [11:0] dst2_s4l;
wire logic signed   [11:0] dst3_s4l;
wire logic signed   [11:0] dst4_s4l;
wire logic signed   [11:0] dst5_s4l;
wire logic signed   [11:0] dst6_s4l;
wire logic signed   [11:0] dst7_s4l;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
cp_r cp1_r(dst1_r, src_r);
cp_r cp2_r(dst2_r, src_u2);
cp_r cp3_r(dst3_r, src_s2);
cp_r cp4_r(dst4_r, src_u4);
cp_r cp5_r(dst5_r, src_s4);
cp_r cp6_r(dst6_r, src_ux);
cp_r cp7_r(dst7_r, src_sx);
`endif

`ifdef SUPPORT_TWO_STATE_NETS_IN_IVTEST
cp_u2s cp1_u2s(dst1_u2s, src_r);
cp_u2s cp2_u2s(dst2_u2s, src_u2);
cp_u2s cp3_u2s(dst3_u2s, src_s2);
cp_u2s cp4_u2s(dst4_u2s, src_u4);
cp_u2s cp5_u2s(dst5_u2s, src_s4);
cp_u2s cp6_u2s(dst6_u2s, src_ux);
cp_u2s cp7_u2s(dst7_u2s, src_sx);

cp_s2s cp1_s2s(dst1_s2s, src_r);
cp_s2s cp2_s2s(dst2_s2s, src_u2);
cp_s2s cp3_s2s(dst3_s2s, src_s2);
cp_s2s cp4_s2s(dst4_s2s, src_u4);
cp_s2s cp5_s2s(dst5_s2s, src_s4);
cp_s2s cp6_s2s(dst6_s2s, src_ux);
cp_s2s cp7_s2s(dst7_s2s, src_sx);

cp_u2l cp1_u2l(dst1_u2l, src_r);
cp_u2l cp2_u2l(dst2_u2l, src_u2);
cp_u2l cp3_u2l(dst3_u2l, src_s2);
cp_u2l cp4_u2l(dst4_u2l, src_u4);
cp_u2l cp5_u2l(dst5_u2l, src_s4);
cp_u2l cp6_u2l(dst6_u2l, src_ux);
cp_u2l cp7_u2l(dst7_u2l, src_sx);

cp_s2l cp1_s2l(dst1_s2l, src_r);
cp_s2l cp2_s2l(dst2_s2l, src_u2);
cp_s2l cp3_s2l(dst3_s2l, src_s2);
cp_s2l cp4_s2l(dst4_s2l, src_u4);
cp_s2l cp5_s2l(dst5_s2l, src_s4);
cp_s2l cp6_s2l(dst6_s2l, src_ux);
cp_s2l cp7_s2l(dst7_s2l, src_sx);
`endif

cp_u4s cp1_u4s(dst1_u4s, src_r);
cp_u4s cp2_u4s(dst2_u4s, src_u2);
cp_u4s cp3_u4s(dst3_u4s, src_s2);
cp_u4s cp4_u4s(dst4_u4s, src_u4);
cp_u4s cp5_u4s(dst5_u4s, src_s4);
cp_u4s cp6_u4s(dst6_u4s, src_ux);
cp_u4s cp7_u4s(dst7_u4s, src_sx);

cp_s4s cp1_s4s(dst1_s4s, src_r);
cp_s4s cp2_s4s(dst2_s4s, src_u2);
cp_s4s cp3_s4s(dst3_s4s, src_s2);
cp_s4s cp4_s4s(dst4_s4s, src_u4);
cp_s4s cp5_s4s(dst5_s4s, src_s4);
cp_s4s cp6_s4s(dst6_s4s, src_ux);
cp_s4s cp7_s4s(dst7_s4s, src_sx);

cp_u4l cp1_u4l(dst1_u4l, src_r);
cp_u4l cp2_u4l(dst2_u4l, src_u2);
cp_u4l cp3_u4l(dst3_u4l, src_s2);
cp_u4l cp4_u4l(dst4_u4l, src_u4);
cp_u4l cp5_u4l(dst5_u4l, src_s4);
cp_u4l cp6_u4l(dst6_u4l, src_ux);
cp_u4l cp7_u4l(dst7_u4l, src_sx);

cp_s4l cp1_s4l(dst1_s4l, src_r);
cp_s4l cp2_s4l(dst2_s4l, src_u2);
cp_s4l cp3_s4l(dst3_s4l, src_s2);
cp_s4l cp4_s4l(dst4_s4l, src_u4);
cp_s4l cp5_s4l(dst5_s4l, src_s4);
cp_s4l cp6_s4l(dst6_s4l, src_ux);
cp_s4l cp7_s4l(dst7_s4l, src_sx);

bit failed;

initial begin
  failed = 0;

  src_r  = -7;
  src_u2 =  7;
  src_s2 = -7;
  src_u4 =  7;
  src_s4 = -7;
  src_ux = 8'bx0z00111;
  src_sx = 8'bx0z00111;

  #1;

`ifdef SUPPORT_REAL_NETS_IN_IVTEST
  $display("cast to real");
  $display("%g", dst1_r); if (dst1_r != -7.0) failed = 1;
  $display("%g", dst2_r); if (dst2_r !=  7.0) failed = 1;
  $display("%g", dst3_r); if (dst3_r != -7.0) failed = 1;
  $display("%g", dst4_r); if (dst4_r !=  7.0) failed = 1;
  $display("%g", dst5_r); if (dst5_r != -7.0) failed = 1;
  $display("%g", dst6_r); if (dst6_r !=  7.0) failed = 1;
  $display("%g", dst7_r); if (dst7_r !=  7.0) failed = 1;
`endif

`ifdef SUPPORT_TWO_STATE_NETS_IN_IVTEST
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
`endif

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
