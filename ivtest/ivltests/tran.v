module test();

reg a, b;

wire a1, a2, a3, a4, a5, a6, a7;

assign (supply1, supply0) a1 = a;

tran t1(a1, a2);
tran t2(a2, a3);
tran t3(a3, a4);
tran t4(a4, a5);
tran t5(a5, a6);
tran t6(a6, a7);

wire a11, a12, a13, a14, a15, b11, b12, b13, b14, b15;
wire a21, a22, a23, a24, a25, b21, b22, b23, b24, b25;
wire a31, a32, a33, a34, a35, b31, b32, b33, b34, b35;
wire a41, a42, a43, a44, a45, b41, b42, b43, b44, b45;
wire a51, a52, a53, a54, a55, b51, b52, b53, b54, b55;

assign (supply1, supply0) a11 = a,  b11 = b;
assign (supply1, strong0) a12 = a,  b12 = b;
assign (supply1,   pull0) a13 = a,  b13 = b;
assign (supply1,   weak0) a14 = a,  b14 = b;
assign (supply1,  highz0) a15 = a,  b15 = b;

assign (strong1, supply0) a21 = a,  b21 = b;
assign (strong1, strong0) a22 = a,  b22 = b;
assign (strong1,   pull0) a23 = a,  b23 = b;
assign (strong1,   weak0) a24 = a,  b24 = b;
assign (strong1,  highz0) a25 = a,  b25 = b;

assign (  pull1, supply0) a31 = a,  b31 = b;
assign (  pull1, strong0) a32 = a,  b32 = b;
assign (  pull1,   pull0) a33 = a,  b33 = b;
assign (  pull1,   weak0) a34 = a,  b34 = b;
assign (  pull1,  highz0) a35 = a,  b35 = b;

assign (  weak1, supply0) a41 = a,  b41 = b;
assign (  weak1, strong0) a42 = a,  b42 = b;
assign (  weak1,   pull0) a43 = a,  b43 = b;
assign (  weak1,   weak0) a44 = a,  b44 = b;
assign (  weak1,  highz0) a45 = a,  b45 = b;

assign ( highz1, supply0) a51 = a,  b51 = b;
assign ( highz1, strong0) a52 = a,  b52 = b;
assign ( highz1,   pull0) a53 = a,  b53 = b;
assign ( highz1,   weak0) a54 = a,  b54 = b;

tran t11(a11, b11);
tran t12(a12, b12);
tran t13(a13, b13);
tran t14(a14, b14);
tran t15(a15, b15);

tran t21(a21, b21);
tran t22(a22, b22);
tran t23(a23, b23);
tran t24(a24, b24);
tran t25(a25, b25);

tran t31(a31, b31);
tran t32(a32, b32);
tran t33(a33, b33);
tran t34(a34, b34);
tran t35(a35, b35);

tran t41(a41, b41);
tran t42(a42, b42);
tran t43(a43, b43);
tran t44(a44, b44);
tran t45(a45, b45);

tran t51(a51, b51);
tran t52(a52, b52);
tran t53(a53, b53);
tran t54(a54, b54);
tran t55(a55, b55);

task display_strengths;

input ta, tb;

begin
  a = ta;
  b = tb;
  #1;
  $display("a = %b b = %b", a, b);
  $display("a1(%v) a2(%v) a3(%v) a4(%v) a5(%v) a6(%v) a7(%v)", a1, a2, a3, a4, a5, a6, a7);
  $display("t11(%v %v) t12(%v %v) t13(%v %v) t14(%v %v) t15(%v %v)", a11, b11, a12, b12, a13, b13, a14, b14, a15, b15);
  $display("t21(%v %v) t22(%v %v) t23(%v %v) t24(%v %v) t25(%v %v)", a21, b21, a22, b22, a23, b23, a24, b24, a25, b25);
  $display("t31(%v %v) t32(%v %v) t33(%v %v) t34(%v %v) t35(%v %v)", a31, b31, a32, b32, a33, b33, a34, b34, a35, b35);
  $display("t41(%v %v) t42(%v %v) t43(%v %v) t44(%v %v) t45(%v %v)", a41, b41, a42, b42, a43, b43, a44, b44, a45, b45);
  $display("t51(%v %v) t52(%v %v) t53(%v %v) t54(%v %v) t55(%v %v)", a51, b51, a52, b52, a53, b53, a54, b54, a55, b55);
end

endtask

initial begin
  display_strengths(1'bz, 1'bz);
  display_strengths(1'bx, 1'bz);
  display_strengths(1'b0, 1'bz);
  display_strengths(1'b1, 1'bz);

  display_strengths(1'bz, 1'bx);
  display_strengths(1'bx, 1'bx);
  display_strengths(1'b0, 1'bx);
  display_strengths(1'b1, 1'bx);

  display_strengths(1'bz, 1'b0);
  display_strengths(1'bx, 1'b0);
  display_strengths(1'b0, 1'b0);
  display_strengths(1'b1, 1'b0);

  display_strengths(1'bz, 1'b1);
  display_strengths(1'bx, 1'b1);
  display_strengths(1'b0, 1'b1);
  display_strengths(1'b1, 1'b1);
end

endmodule
