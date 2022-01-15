`ifdef __ICARUS__
  `define SUPPORT_REAL_MODULUS_IN_IVTEST
`endif

module test();

function real pre_inc(input real x);
begin
  ++x;
  pre_inc = x;
end
endfunction

function real pre_dec(input real x);
begin
  --x;
  pre_dec = x;
end
endfunction

function real post_inc(input real x);
begin
  x++;
  post_inc = x;
end
endfunction

function real post_dec(input real x);
begin
  x--;
  post_dec = x;
end
endfunction

localparam pre_inc_5 = pre_inc(5);
localparam pre_dec_5 = pre_dec(5);

localparam post_inc_5 = post_inc(5);
localparam post_dec_5 = post_dec(5);

function real add2(input real x);
begin
  x += 2;
  add2 = x;
end
endfunction

function real sub2(input real x);
begin
  x -= 2;
  sub2 = x;
end
endfunction

function real mul2(input real x);
begin
  x *= 2;
  mul2 = x;
end
endfunction

function real div2(input real x);
begin
  x /= 2;
  div2 = x;
end
endfunction

`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
function real mod2(input real x);
begin
  x %= 2;
  mod2 = x;
end
endfunction
`endif

localparam add2_5 = add2(5);
localparam sub2_5 = sub2(5);
localparam mul2_5 = mul2(5);
localparam div2_5 = div2(5);
`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
localparam mod2_5 = mod2(5);
`endif

function real add3(input real x);
begin
  add3 = x;
  add3 += 3;
end
endfunction

function real sub3(input real x);
begin
  sub3 = x;
  sub3 -= 3;
end
endfunction

function real mul3(input real x);
begin
  mul3 = x;
  mul3 *= 3;
end
endfunction

function real div4(input real x);
begin
  div4 = x;
  div4 /= 4;
end
endfunction

`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
function real mod3(input real x);
begin
  mod3 = x;
  mod3 %= 3;
end
endfunction
`endif

localparam add3_5 = add3(5);
localparam sub3_5 = sub3(5);
localparam mul3_5 = mul3(5);
localparam div4_5 = div4(5);
`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
localparam mod3_5 = mod3(5);
`endif

reg failed = 0;

initial begin
  $display("pre_inc_5 = %0f", pre_inc_5);
  if (pre_inc_5 != pre_inc(5)) failed = 1;
  if (pre_inc_5 != 6.0) failed = 1;

  $display("pre_dec_5 = %0f", pre_dec_5);
  if (pre_dec_5 != pre_dec(5)) failed = 1;
  if (pre_dec_5 != 4.0) failed = 1;

  $display("post_inc_5 = %0f", post_inc_5);
  if (post_inc_5 != post_inc(5)) failed = 1;
  if (post_inc_5 != 6.0) failed = 1;

  $display("post_dec_5 = %0f", post_dec_5);
  if (post_dec_5 != post_dec(5)) failed = 1;
  if (post_dec_5 != 4.0) failed = 1;

  $display("add2_5 = %0f", add2_5);
  if (add2_5 != add2(5)) failed = 1;
  if (add2_5 != 7.0) failed = 1;

  $display("sub2_5 = %0f", sub2_5);
  if (sub2_5 != sub2(5)) failed = 1;
  if (sub2_5 != 3.0) failed = 1;

  $display("mul2_5 = %0f", mul2_5);
  if (mul2_5 != mul2(5)) failed = 1;
  if (mul2_5 != 10.0) failed = 1;

  $display("div2_5 = %0f", div2_5);
  if (div2_5 != div2(5)) failed = 1;
  if (div2_5 != 2.5) failed = 1;

`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
  $display("mod2_5 = %0f", mod2_5);
  if (mod2_5 != mod2(5)) failed = 1;
  if (mod2_5 != 1.0) failed = 1;
`endif

  $display("add3_5 = %0f", add3_5);
  if (add3_5 != add3(5)) failed = 1;
  if (add3_5 != 8.0) failed = 1;

  $display("sub3_5 = %0f", sub3_5);
  if (sub3_5 != sub3(5)) failed = 1;
  if (sub3_5 != 2.0) failed = 1;

  $display("mul3_5 = %0f", mul3_5);
  if (mul3_5 != mul3(5)) failed = 1;
  if (mul3_5 != 15.0) failed = 1;

  $display("div4_5 = %0f", div4_5);
  if (div4_5 != div4(5)) failed = 1;
  if (div4_5 != 1.25) failed = 1;

`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
  $display("mod3_5 = %0f", mod3_5);
  if (mod3_5 != mod3(5)) failed = 1;
  if (mod3_5 != 2.0) failed = 1;
`endif

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
