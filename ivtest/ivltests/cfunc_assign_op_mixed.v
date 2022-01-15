`ifdef __ICARUS__
  `define SUPPORT_REAL_MODULUS_IN_IVTEST
`endif

module test();

function integer add2(input integer x);
begin
  x += 2.0;
  add2 = x;
end
endfunction

function integer sub2(input integer x);
begin
  x -= 2.0;
  sub2 = x;
end
endfunction

function integer mul2(input integer x);
begin
  x *= 2.0;
  mul2 = x;
end
endfunction

function integer div2(input integer x);
begin
  x /= 2.0;
  div2 = x;
end
endfunction

function integer mod2(input integer x);
begin
`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
  x %= 2.0;
`else
  x %= 2;
`endif
  mod2 = x;
end
endfunction

localparam add2_5 = add2(5);
localparam sub2_5 = sub2(5);
localparam mul2_5 = mul2(5);
localparam div2_5 = div2(5);
localparam mod2_5 = mod2(5);

function integer add3(input integer x);
begin
  add3 = x;
  add3 += 3.0;
end
endfunction

function integer sub3(input integer x);
begin
  sub3 = x;
  sub3 -= 3.0;
end
endfunction

function integer mul3(input integer x);
begin
  mul3 = x;
  mul3 *= 3.0;
end
endfunction

function integer div3(input integer x);
begin
  div3 = x;
  div3 /= 3.0;
end
endfunction

function integer mod3(input integer x);
begin
  mod3 = x;
`ifdef SUPPORT_REAL_MODULUS_IN_IVTEST
  mod3 %= 3.0;
`else
  mod3 %= 3;
`endif
end
endfunction

localparam add3_5 = add3(5);
localparam sub3_5 = sub3(5);
localparam mul3_5 = mul3(5);
localparam div3_5 = div3(5);
localparam mod3_5 = mod3(5);

reg failed = 0;

initial begin
  $display("add2_5 = %0f", add2_5);
  if (add2_5 != add2(5)) failed = 1;
  if (add2_5 != 7) failed = 1;

  $display("sub2_5 = %0f", sub2_5);
  if (sub2_5 != sub2(5)) failed = 1;
  if (sub2_5 != 3) failed = 1;

  $display("mul2_5 = %0f", mul2_5);
  if (mul2_5 != mul2(5)) failed = 1;
  if (mul2_5 != 10) failed = 1;

  $display("div2_5 = %0f", div2_5);
  if (div2_5 != div2(5)) failed = 1;
  if (div2_5 != 2) failed = 1;

  $display("mod2_5 = %0f", mod2_5);
  if (mod2_5 != mod2(5)) failed = 1;
  if (mod2_5 != 1) failed = 1;

  $display("add3_5 = %0f", add3_5);
  if (add3_5 != add3(5)) failed = 1;
  if (add3_5 != 8) failed = 1;

  $display("sub3_5 = %0f", sub3_5);
  if (sub3_5 != sub3(5)) failed = 1;
  if (sub3_5 != 2) failed = 1;

  $display("mul3_5 = %0f", mul3_5);
  if (mul3_5 != mul3(5)) failed = 1;
  if (mul3_5 != 15) failed = 1;

  $display("div3_5 = %0f", div3_5);
  if (div3_5 != div3(5)) failed = 1;
  if (div3_5 != 1) failed = 1;

  $display("mod3_5 = %0f", mod3_5);
  if (mod3_5 != mod3(5)) failed = 1;
  if (mod3_5 != 2) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
