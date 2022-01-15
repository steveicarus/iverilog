// Test implicit casts during procedural blocking assignments.

module implicit_cast();

real                  src_r;

bit   unsigned  [7:0] src_u2;
bit   signed    [7:0] src_s2;

logic unsigned  [7:0] src_u4;
logic signed    [7:0] src_s4;

logic unsigned  [7:0] src_ux;
logic signed    [7:0] src_sx;

real                  dst_r;

bit   unsigned  [3:0] dst_u2s;
bit   signed    [3:0] dst_s2s;

bit   unsigned [11:0] dst_u2l;
bit   signed   [11:0] dst_s2l;

logic unsigned  [3:0] dst_u4s;
logic signed    [3:0] dst_s4s;

logic unsigned [11:0] dst_u4l;
logic signed   [11:0] dst_s4l;

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

  $display("cast to real");
  dst_r = src_r;  $display("%g", dst_r); if (dst_r != -7.0) failed = 1;
  dst_r = src_u2; $display("%g", dst_r); if (dst_r !=  7.0) failed = 1;
  dst_r = src_s2; $display("%g", dst_r); if (dst_r != -7.0) failed = 1;
  dst_r = src_u4; $display("%g", dst_r); if (dst_r !=  7.0) failed = 1;
  dst_r = src_s4; $display("%g", dst_r); if (dst_r != -7.0) failed = 1;
  dst_r = src_ux; $display("%g", dst_r); if (dst_r !=  7.0) failed = 1;
  dst_r = src_sx; $display("%g", dst_r); if (dst_r !=  7.0) failed = 1;

  $display("cast to small unsigned bit");
  dst_u2s = src_r;  $display("%d", dst_u2s); if (dst_u2s !== 4'd9) failed = 1;
  dst_u2s = src_u2; $display("%d", dst_u2s); if (dst_u2s !== 4'd7) failed = 1;
  dst_u2s = src_s2; $display("%d", dst_u2s); if (dst_u2s !== 4'd9) failed = 1;
  dst_u2s = src_u4; $display("%d", dst_u2s); if (dst_u2s !== 4'd7) failed = 1;
  dst_u2s = src_s4; $display("%d", dst_u2s); if (dst_u2s !== 4'd9) failed = 1;
  dst_u2s = src_ux; $display("%d", dst_u2s); if (dst_u2s !== 4'd7) failed = 1;
  dst_u2s = src_sx; $display("%d", dst_u2s); if (dst_u2s !== 4'd7) failed = 1;

  $display("cast to small signed bit");
  dst_s2s = src_r;  $display("%d", dst_s2s); if (dst_s2s !== -4'sd7) failed = 1;
  dst_s2s = src_u2; $display("%d", dst_s2s); if (dst_s2s !==  4'sd7) failed = 1;
  dst_s2s = src_s2; $display("%d", dst_s2s); if (dst_s2s !== -4'sd7) failed = 1;
  dst_s2s = src_u4; $display("%d", dst_s2s); if (dst_s2s !==  4'sd7) failed = 1;
  dst_s2s = src_s4; $display("%d", dst_s2s); if (dst_s2s !== -4'sd7) failed = 1;
  dst_s2s = src_ux; $display("%d", dst_s2s); if (dst_s2s !==  4'sd7) failed = 1;
  dst_s2s = src_sx; $display("%d", dst_s2s); if (dst_s2s !==  4'sd7) failed = 1;

  $display("cast to large unsigned bit");
  dst_u2l = src_r;  $display("%d", dst_u2l); if (dst_u2l !== 12'd4089) failed = 1;
  dst_u2l = src_u2; $display("%d", dst_u2l); if (dst_u2l !== 12'd7)    failed = 1;
  dst_u2l = src_s2; $display("%d", dst_u2l); if (dst_u2l !== 12'd4089) failed = 1;
  dst_u2l = src_u4; $display("%d", dst_u2l); if (dst_u2l !== 12'd7)    failed = 1;
  dst_u2l = src_s4; $display("%d", dst_u2l); if (dst_u2l !== 12'd4089) failed = 1;
  dst_u2l = src_ux; $display("%b", dst_u2l); if (dst_u2l !== 12'b000000000111) failed = 1;
  dst_u2l = src_sx; $display("%b", dst_u2l); if (dst_u2l !== 12'b000000000111) failed = 1;

  $display("cast to large signed bit");
  dst_s2l = src_r;  $display("%d", dst_s2l); if (dst_s2l !== -12'sd7) failed = 1;
  dst_s2l = src_u2; $display("%d", dst_s2l); if (dst_s2l !==  12'sd7) failed = 1;
  dst_s2l = src_s2; $display("%d", dst_s2l); if (dst_s2l !== -12'sd7) failed = 1;
  dst_s2l = src_u4; $display("%d", dst_s2l); if (dst_s2l !==  12'sd7) failed = 1;
  dst_s2l = src_s4; $display("%d", dst_s2l); if (dst_s2l !== -12'sd7) failed = 1;
  dst_s2l = src_ux; $display("%b", dst_s2l); if (dst_s2l !== 12'b000000000111) failed = 1;
  dst_s2l = src_sx; $display("%b", dst_s2l); if (dst_s2l !== 12'b000000000111) failed = 1;

  $display("cast to small unsigned logic");
  dst_u4s = src_r;  $display("%d", dst_u4s); if (dst_u4s !== 4'd9) failed = 1;
  dst_u4s = src_u2; $display("%d", dst_u4s); if (dst_u4s !== 4'd7) failed = 1;
  dst_u4s = src_s2; $display("%d", dst_u4s); if (dst_u4s !== 4'd9) failed = 1;
  dst_u4s = src_u4; $display("%d", dst_u4s); if (dst_u4s !== 4'd7) failed = 1;
  dst_u4s = src_s4; $display("%d", dst_u4s); if (dst_u4s !== 4'd9) failed = 1;
  dst_u4s = src_ux; $display("%d", dst_u4s); if (dst_u4s !== 4'd7) failed = 1;
  dst_u4s = src_sx; $display("%d", dst_u4s); if (dst_u4s !== 4'd7) failed = 1;

  $display("cast to small signed logic");
  dst_s4s = src_r;  $display("%d", dst_s4s); if (dst_s4s !== -4'sd7) failed = 1;
  dst_s4s = src_u2; $display("%d", dst_s4s); if (dst_s4s !==  4'sd7) failed = 1;
  dst_s4s = src_s2; $display("%d", dst_s4s); if (dst_s4s !== -4'sd7) failed = 1;
  dst_s4s = src_u4; $display("%d", dst_s4s); if (dst_s4s !==  4'sd7) failed = 1;
  dst_s4s = src_s4; $display("%d", dst_s4s); if (dst_s4s !== -4'sd7) failed = 1;
  dst_s4s = src_ux; $display("%d", dst_s4s); if (dst_s4s !==  4'sd7) failed = 1;
  dst_s4s = src_sx; $display("%d", dst_s4s); if (dst_s4s !==  4'sd7) failed = 1;

  $display("cast to large unsigned logic");
  dst_u4l = src_r;  $display("%d", dst_u4l); if (dst_u4l !== 12'd4089) failed = 1;
  dst_u4l = src_u2; $display("%d", dst_u4l); if (dst_u4l !== 12'd7)    failed = 1;
  dst_u4l = src_s2; $display("%d", dst_u4l); if (dst_u4l !== 12'd4089) failed = 1;
  dst_u4l = src_u4; $display("%d", dst_u4l); if (dst_u4l !== 12'd7)    failed = 1;
  dst_u4l = src_s4; $display("%d", dst_u4l); if (dst_u4l !== 12'd4089) failed = 1;
  dst_u4l = src_ux; $display("%b", dst_u4l); if (dst_u4l !== 12'b0000x0z00111) failed = 1;
  dst_u4l = src_sx; $display("%b", dst_u4l); if (dst_u4l !== 12'bxxxxx0z00111) failed = 1;

  $display("cast to large signed logic");
  dst_s4l = src_r;  $display("%d", dst_s4l); if (dst_s4l !== -12'sd7) failed = 1;
  dst_s4l = src_u2; $display("%d", dst_s4l); if (dst_s4l !==  12'sd7) failed = 1;
  dst_s4l = src_s2; $display("%d", dst_s4l); if (dst_s4l !== -12'sd7) failed = 1;
  dst_s4l = src_u4; $display("%d", dst_s4l); if (dst_s4l !==  12'sd7) failed = 1;
  dst_s4l = src_s4; $display("%d", dst_s4l); if (dst_s4l !== -12'sd7) failed = 1;
  dst_s4l = src_ux; $display("%b", dst_s4l); if (dst_s4l !==  12'b0000x0z00111) failed = 1;
  dst_s4l = src_sx; $display("%b", dst_s4l); if (dst_s4l !==  12'bxxxxx0z00111) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
