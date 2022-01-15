module test();

function integer pre_inc(input integer x);
begin
  ++x;
  pre_inc = x;
end
endfunction

function integer pre_dec(input integer x);
begin
  --x;
  pre_dec = x;
end
endfunction

function integer post_inc(input integer x);
begin
  x++;
  post_inc = x;
end
endfunction

function integer post_dec(input integer x);
begin
  x--;
  post_dec = x;
end
endfunction

localparam pre_inc_5 = pre_inc(5);
localparam pre_dec_5 = pre_dec(5);

localparam post_inc_5 = post_inc(5);
localparam post_dec_5 = post_dec(5);

function integer add2(input integer x);
begin
  x += 2;
  add2 = x;
end
endfunction

function integer sub2(input integer x);
begin
  x -= 2;
  sub2 = x;
end
endfunction

function integer mul2(input integer x);
begin
  x *= 2;
  mul2 = x;
end
endfunction

function integer div2(input integer x);
begin
  x /= 2;
  div2 = x;
end
endfunction

function integer mod2(input integer x);
begin
  x %= 2;
  mod2 = x;
end
endfunction

function [3:0] and6(input [3:0] x);
begin
  x &= 4'h6;
  and6 = x;
end
endfunction

function [3:0] or6(input [3:0] x);
begin
  x |= 4'h6;
  or6 = x;
end
endfunction

function [3:0] xor6(input [3:0] x);
begin
  x ^= 4'h6;
  xor6 = x;
end
endfunction

function integer lsl2(input integer x);
begin
  x <<= 2;
  lsl2 = x;
end
endfunction

function integer lsr2(input integer x);
begin
  x >>= 2;
  lsr2 = x;
end
endfunction

function integer asl2(input integer x);
begin
  x <<<= 2;
  asl2 = x;
end
endfunction

function integer asr2(input integer x);
begin
  x >>>= 2;
  asr2 = x;
end
endfunction

localparam add2_5 = add2(5);
localparam sub2_5 = sub2(5);
localparam mul2_5 = mul2(5);
localparam div2_5 = div2(5);
localparam mod2_5 = mod2(5);

localparam and6_f = and6(4'hf);
localparam  or6_0 =  or6(4'h0);
localparam xor6_f = xor6(4'hf);

localparam lsl2_p25 = lsl2( 25);
localparam lsr2_m25 = lsr2(-25);
localparam asl2_m25 = asl2(-25);
localparam asr2_m25 = asr2(-25);

function integer add3(input integer x);
begin
  add3 = x;
  add3 += 3;
end
endfunction

function integer sub3(input integer x);
begin
  sub3 = x;
  sub3 -= 3;
end
endfunction

function integer mul3(input integer x);
begin
  mul3 = x;
  mul3 *= 3;
end
endfunction

function integer div3(input integer x);
begin
  div3 = x;
  div3 /= 3;
end
endfunction

function integer mod3(input integer x);
begin
  mod3 = x;
  mod3 %= 3;
end
endfunction

function [3:0] and9(input [3:0] x);
begin
  and9 = x;
  and9 &= 4'h9;
end
endfunction

function [3:0] or9(input [3:0] x);
begin
  or9 = x;
  or9 |= 4'h9;
end
endfunction

function [3:0] xor9(input [3:0] x);
begin
  xor9 = x;
  xor9 ^= 4'h9;
end
endfunction

function integer lsl3(input integer x);
begin
  lsl3 = x;
  lsl3 <<= 3;
end
endfunction

function integer lsr3(input integer x);
begin
  lsr3 = x;
  lsr3 >>= 3;
end
endfunction

function integer asl3(input integer x);
begin
  asl3 = x;
  asl3 <<<= 3;
end
endfunction

function integer asr3(input integer x);
begin
  asr3 = x;
  asr3 >>>= 3;
end
endfunction

localparam add3_5 = add3(5);
localparam sub3_5 = sub3(5);
localparam mul3_5 = mul3(5);
localparam div3_5 = div3(5);
localparam mod3_5 = mod3(5);

localparam and9_f = and9(4'hf);
localparam  or9_0 =  or9(4'h0);
localparam xor9_f = xor9(4'hf);

localparam lsl3_p25 = lsl3( 25);
localparam lsr3_m25 = lsr3(-25);
localparam asl3_m25 = asl3(-25);
localparam asr3_m25 = asr3(-25);

reg failed = 0;

initial begin
  $display("pre_inc_5 = %0d", pre_inc_5);
  if (pre_inc_5 !== pre_inc(5)) failed = 1;
  if (pre_inc_5 !== 6) failed = 1;

  $display("pre_dec_5 = %0d", pre_dec_5);
  if (pre_dec_5 !== pre_dec(5)) failed = 1;
  if (pre_dec_5 !== 4) failed = 1;

  $display("post_inc_5 = %0d", post_inc_5);
  if (post_inc_5 !== post_inc(5)) failed = 1;
  if (post_inc_5 !== 6) failed = 1;

  $display("post_dec_5 = %0d", post_dec_5);
  if (post_dec_5 !== post_dec(5)) failed = 1;
  if (post_dec_5 !== 4) failed = 1;

  $display("add2_5 = %0d", add2_5);
  if (add2_5 !== add2(5)) failed = 1;
  if (add2_5 !== 7) failed = 1;

  $display("sub2_5 = %0d", sub2_5);
  if (sub2_5 !== sub2(5)) failed = 1;
  if (sub2_5 !== 3) failed = 1;

  $display("mul2_5 = %0d", mul2_5);
  if (mul2_5 !== mul2(5)) failed = 1;
  if (mul2_5 !== 10) failed = 1;

  $display("div2_5 = %0d", div2_5);
  if (div2_5 !== div2(5)) failed = 1;
  if (div2_5 !== 2) failed = 1;

  $display("mod2_5 = %0d", mod2_5);
  if (mod2_5 !== mod2(5)) failed = 1;
  if (mod2_5 !== 1) failed = 1;

  $display("and6_f = %h", and6_f);
  if (and6_f !== and6(4'hf)) failed = 1;
  if (and6_f !== 4'h6) failed = 1;

  $display("or6_0 = %h", or6_0);
  if (or6_0 !==  or6(4'h0)) failed = 1;
  if (or6_0 !==  4'h6) failed = 1;

  $display("xor6_f = %h", xor6_f);
  if (xor6_f !== xor6(4'hf)) failed = 1;
  if (xor6_f !== 4'h9) failed = 1;

  $display("lsl2_p25 = %0d", lsl2_p25);
  if (lsl2_p25 !== lsl2( 25)) failed = 1;
  if (lsl2_p25 !== 100) failed = 1;

  $display("lsr2_m25 = %0h", lsr2_m25);
  if (lsr2_m25 !== lsr2(-25)) failed = 1;
  if (lsr2_m25 !== 32'h3ffffff9) failed = 1;

  $display("asl2_m25 = %0d", asl2_m25);
  if (asl2_m25 !== asl2(-25)) failed = 1;
  if (asl2_m25 !== -100) failed = 1;

  $display("asr2_m25 = %0d", asr2_m25);
  if (asr2_m25 !== asr2(-25)) failed = 1;
  if (asr2_m25 !== -7) failed = 1;

  $display("add3_5 = %0d", add3_5);
  if (add3_5 !== add3(5)) failed = 1;
  if (add3_5 !== 8) failed = 1;

  $display("sub3_5 = %0d", sub3_5);
  if (sub3_5 !== sub3(5)) failed = 1;
  if (sub3_5 !== 2) failed = 1;

  $display("mul3_5 = %0d", mul3_5);
  if (mul3_5 !== mul3(5)) failed = 1;
  if (mul3_5 !== 15) failed = 1;

  $display("div3_5 = %0d", div3_5);
  if (div3_5 !== div3(5)) failed = 1;
  if (div3_5 !== 1) failed = 1;

  $display("mod3_5 = %0d", mod3_5);
  if (mod3_5 !== mod3(5)) failed = 1;
  if (mod3_5 !== 2) failed = 1;

  $display("and9_f = %h", and9_f);
  if (and9_f !== and9(4'hf)) failed = 1;
  if (and9_f !== 4'h9) failed = 1;

  $display("or9_0 = %h", or9_0);
  if (or9_0 !==  or9(4'h0)) failed = 1;
  if (or9_0 !==  4'h9) failed = 1;

  $display("xor9_f = %h", xor9_f);
  if (xor9_f !== xor9(4'hf)) failed = 1;
  if (xor9_f !== 4'h6) failed = 1;

  $display("lsl3_p25 = %0d", lsl3_p25);
  if (lsl3_p25 !== lsl3( 25)) failed = 1;
  if (lsl3_p25 !== 200) failed = 1;

  $display("lsr3_m25 = %0h", lsr3_m25);
  if (lsr3_m25 !== lsr3(-25)) failed = 1;
  if (lsr3_m25 !== 32'h1ffffffc) failed = 1;

  $display("asl3_m25 = %0d", asl3_m25);
  if (asl3_m25 !== asl3(-25)) failed = 1;
  if (asl3_m25 !== -200) failed = 1;

  $display("asr3_m25 = %0d", asr3_m25);
  if (asr3_m25 !== asr3(-25)) failed = 1;
  if (asr3_m25 !== -4) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
