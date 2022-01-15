module test();

function [31:0] pre_inc(input [31:0] x);
begin
  ++x[23:8];
  pre_inc = x;
end
endfunction

function [31:0] pre_dec(input [31:0] x);
begin
  --x[23:8];
  pre_dec = x;
end
endfunction

function [31:0] post_inc(input [31:0] x);
begin
  x[23:8]++;
  post_inc = x;
end
endfunction

function [31:0] post_dec(input [31:0] x);
begin
  x[23:8]--;
  post_dec = x;
end
endfunction

localparam pre_inc_5 = pre_inc({8'h55, 16'd5, 8'haa});
localparam pre_dec_5 = pre_dec({8'h55, 16'd5, 8'haa});

localparam post_inc_5 = post_inc({8'h55, 16'd5, 8'haa});
localparam post_dec_5 = post_dec({8'h55, 16'd5, 8'haa});

function [31:0] add2(input [31:0] x);
begin
  x[23:8] += 2;
  add2 = x;
end
endfunction

function [31:0] sub2(input [31:0] x);
begin
  x[23:8] -= 2;
  sub2 = x;
end
endfunction

function [31:0] mul2(input [31:0] x);
begin
  x[23:8] *= 2;
  mul2 = x;
end
endfunction

function [31:0] div2(input [31:0] x);
begin
  x[23:8] /= 2;
  div2 = x;
end
endfunction

function [31:0] mod2(input [31:0] x);
begin
  x[23:8] %= 2;
  mod2 = x;
end
endfunction

function [31:0] and6(input [31:0] x);
begin
  x[23:8] &= 16'h6666;
  and6 = x;
end
endfunction

function [31:0] or6(input [31:0] x);
begin
  x[23:8] |= 16'h6666;
  or6 = x;
end
endfunction

function [31:0] xor6(input [31:0] x);
begin
  x[23:8] ^= 16'h6666;
  xor6 = x;
end
endfunction

function [31:0] lsl2(input [31:0] x);
begin
  x[23:8] <<= 2;
  lsl2 = x;
end
endfunction

function [31:0] lsr2(input [31:0] x);
begin
  x[23:8] >>= 2;
  lsr2 = x;
end
endfunction

function [31:0] asl2(input [31:0] x);
begin
  x[23:8] <<<= 2;
  asl2 = x;
end
endfunction

function [31:0] asr2(input [31:0] x);
begin
  x[23:8] >>>= 2;
  asr2 = x;
end
endfunction

localparam add2_5 = add2({8'h55, 16'd5, 8'haa});
localparam sub2_5 = sub2({8'h55, 16'd5, 8'haa});
localparam mul2_5 = mul2({8'h55, 16'd5, 8'haa});
localparam div2_5 = div2({8'h55, 16'd5, 8'haa});
localparam mod2_5 = mod2({8'h55, 16'd5, 8'haa});

localparam and6_f = and6(32'h55ffffaa);
localparam  or6_0 =  or6(32'h550000aa);
localparam xor6_f = xor6(32'h55ffffaa);

localparam lsl2_p25 = lsl2({8'h55,  16'sd25, 8'haa});
localparam lsr2_m25 = lsr2({8'h55, -16'sd25, 8'haa});
localparam asl2_m25 = asl2({8'h55, -16'sd25, 8'haa});
localparam asr2_m25 = asr2({8'h55, -16'sd25, 8'haa});

function [31:0] add3(input [31:0] x);
begin
  add3 = x;
  add3[23:8] += 3;
end
endfunction

function [31:0] sub3(input [31:0] x);
begin
  sub3 = x;
  sub3[23:8] -= 3;
end
endfunction

function [31:0] mul3(input [31:0] x);
begin
  mul3 = x;
  mul3[23:8] *= 3;
end
endfunction

function [31:0] div3(input [31:0] x);
begin
  div3 = x;
  div3[23:8] /= 3;
end
endfunction

function [31:0] mod3(input [31:0] x);
begin
  mod3 = x;
  mod3[23:8] %= 3;
end
endfunction

function [31:0] and9(input [31:0] x);
begin
  and9 = x;
  and9[23:8] &= 16'h9999;
end
endfunction

function [31:0] or9(input [31:0] x);
begin
  or9 = x;
  or9[23:8] |= 16'h9999;
end
endfunction

function [31:0] xor9(input [31:0] x);
begin
  xor9 = x;
  xor9[23:8] ^= 16'h9999;
end
endfunction

function [31:0] lsl3(input [31:0] x);
begin
  lsl3 = x;
  lsl3[23:8] <<= 3;
end
endfunction

function [31:0] lsr3(input [31:0] x);
begin
  lsr3 = x;
  lsr3[23:8] >>= 3;
end
endfunction

function [31:0] asl3(input [31:0] x);
begin
  asl3 = x;
  asl3[23:8] <<<= 3;
end
endfunction

function [31:0] asr3(input [31:0] x);
begin
  asr3 = x;
  asr3[23:8] >>>= 3;
end
endfunction

localparam add3_5 = add3({8'h55, 16'd5, 8'haa});
localparam sub3_5 = sub3({8'h55, 16'd5, 8'haa});
localparam mul3_5 = mul3({8'h55, 16'd5, 8'haa});
localparam div3_5 = div3({8'h55, 16'd5, 8'haa});
localparam mod3_5 = mod3({8'h55, 16'd5, 8'haa});

localparam and9_f = and9(32'h55ffffaa);
localparam  or9_0 =  or9(32'h550000aa);
localparam xor9_f = xor9(32'h55ffffaa);

localparam lsl3_p25 = lsl3({8'h55,  16'sd25, 8'haa});
localparam lsr3_m25 = lsr3({8'h55, -16'sd25, 8'haa});
localparam asl3_m25 = asl3({8'h55, -16'sd25, 8'haa});
localparam asr3_m25 = asr3({8'h55, -16'sd25, 8'haa});

reg failed = 0;

initial begin
  $display("pre_inc_5 = %0h", pre_inc_5);
  if (pre_inc_5 !== pre_inc({8'h55, 16'd5, 8'haa})) failed = 1;
  if (pre_inc_5 !== 32'h550006aa) failed = 1;

  $display("pre_dec_5 = %0h", pre_dec_5);
  if (pre_dec_5 !== pre_dec({8'h55, 16'd5, 8'haa})) failed = 1;
  if (pre_dec_5 !== 32'h550004aa) failed = 1;

  $display("post_inc_5 = %0h", post_inc_5);
  if (post_inc_5 !== post_inc({8'h55, 16'd5, 8'haa})) failed = 1;
  if (post_inc_5 !== 32'h550006aa) failed = 1;

  $display("post_dec_5 = %0h", post_dec_5);
  if (post_dec_5 !== post_dec({8'h55, 16'd5, 8'haa})) failed = 1;
  if (post_dec_5 !== 32'h550004aa) failed = 1;

  $display("add2_5 = %0h", add2_5);
  if (add2_5 !== add2({8'h55, 16'd5, 8'haa})) failed = 1;
  if (add2_5 !== 32'h550007aa) failed = 1;

  $display("sub2_5 = %0h", sub2_5);
  if (sub2_5 !== sub2({8'h55, 16'd5, 8'haa})) failed = 1;
  if (sub2_5 !== 32'h550003aa) failed = 1;

  $display("mul2_5 = %0h", mul2_5);
  if (mul2_5 !== mul2({8'h55, 16'd5, 8'haa})) failed = 1;
  if (mul2_5 !== 32'h55000aaa) failed = 1;

  $display("div2_5 = %0h", div2_5);
  if (div2_5 !== div2({8'h55, 16'd5, 8'haa})) failed = 1;
  if (div2_5 !== 32'h550002aa) failed = 1;

  $display("mod2_5 = %0h", mod2_5);
  if (mod2_5 !== mod2({8'h55, 16'd5, 8'haa})) failed = 1;
  if (mod2_5 !== 32'h550001aa) failed = 1;

  $display("and6_f = %h", and6_f);
  if (and6_f !== and6(32'h55ffffaa)) failed = 1;
  if (and6_f !== 32'h556666aa) failed = 1;

  $display(" or6_0 = %h", or6_0);
  if (or6_0 !==  or6(32'h550000aa)) failed = 1;
  if (or6_0 !==  32'h556666aa) failed = 1;

  $display("xor6_f = %h", xor6_f);
  if (xor6_f !== xor6(32'h55ffffaa)) failed = 1;
  if (xor6_f !== 32'h559999aa) failed = 1;

  $display("lsl2_p25 = %0h", lsl2_p25);
  if (lsl2_p25 !== lsl2({8'h55,  16'sd25, 8'haa})) failed = 1;
  if (lsl2_p25 !== 32'h550064aa) failed = 1;

  $display("lsr2_m25 = %0h", lsr2_m25);
  if (lsr2_m25 !== lsr2({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (lsr2_m25 !== 32'h553ff9aa) failed = 1;

  $display("asl2_m25 = %0h", asl2_m25);
  if (asl2_m25 !== asl2({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (asl2_m25 !== 32'h55ff9caa) failed = 1;

  $display("asr2_m25 = %0h", asr2_m25);
  if (asr2_m25 !== asr2({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (asr2_m25 !== 32'h553ff9aa) failed = 1;

  $display("add3_5 = %0h", add3_5);
  if (add3_5 !== add3({8'h55, 16'd5, 8'haa})) failed = 1;
  if (add3_5 !== 32'h550008aa) failed = 1;

  $display("sub3_5 = %0h", sub3_5);
  if (sub3_5 !== sub3({8'h55, 16'd5, 8'haa})) failed = 1;
  if (sub3_5 !== 32'h550002aa) failed = 1;

  $display("mul3_5 = %0h", mul3_5);
  if (mul3_5 !== mul3({8'h55, 16'd5, 8'haa})) failed = 1;
  if (mul3_5 !== 32'h55000faa) failed = 1;

  $display("div3_5 = %0h", div3_5);
  if (div3_5 !== div3({8'h55, 16'd5, 8'haa})) failed = 1;
  if (div3_5 !== 32'h550001aa) failed = 1;

  $display("mod3_5 = %0h", mod3_5);
  if (mod3_5 !== mod3({8'h55, 16'd5, 8'haa})) failed = 1;
  if (mod3_5 !== 32'h550002aa) failed = 1;

  $display("and9_f = %h", and9_f);
  if (and9_f !== and9(32'h55ffffaa)) failed = 1;
  if (and9_f !== 32'h559999aa) failed = 1;

  $display(" or9_0 = %h", or9_0);
  if (or9_0 !==  or9(32'h550000aa)) failed = 1;
  if (or9_0 !==  32'h559999aa) failed = 1;

  $display("xor9_f = %h", xor9_f);
  if (xor9_f !== xor9(32'h55ffffaa)) failed = 1;
  if (xor9_f !== 32'h556666aa) failed = 1;

  $display("lsl3_p25 = %0h", lsl3_p25);
  if (lsl3_p25 !== lsl3({8'h55,  16'sd25, 8'haa})) failed = 1;
  if (lsl3_p25 !== 32'h5500c8aa) failed = 1;

  $display("lsr3_m25 = %0h", lsr3_m25);
  if (lsr3_m25 !== lsr3({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (lsr3_m25 !== 32'h551ffcaa) failed = 1;

  $display("asl3_m25 = %0h", asl3_m25);
  if (asl3_m25 !== asl3({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (asl3_m25 !== 32'h55ff38aa) failed = 1;

  $display("asr3_m25 = %0h", asr3_m25);
  if (asr3_m25 !== asr3({8'h55, -16'sd25, 8'haa})) failed = 1;
  if (asr3_m25 !== 32'h551ffcaa) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
