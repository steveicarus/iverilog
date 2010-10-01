//
//      Name:                   testbench1.vhdl
//
//      Author:                 Chris Eilbeck, chris@yordas.demon.co.uk
//
//      Purpose:                VHDL testbench for a DES encryptor.
//
//      IP Status:              Free use is hereby granted for all civil use including personal, educational and commercial use.
//                                      The use of this code for military, diplomatic or governmental purposes is specifically forbidden.
//
//      Warranty:               There is absolutely no warranty given with this code.  You accept all responsibility for the use
//                                      of this code and any damage so caused.
//
//      Vers Info:              v0.1 14/11/1998 - Creation.
//				     14/11/1999 - Converted to Verilog (ajb)
//

module top;

reg clk;
reg [1:64] pt, key;
wire [1:64] ct;
integer i;

des des(pt, key, ct, clk);

initial
begin
$dumpfile("des.vcd");
$dumpvars(0, top);

key = 64'h0000000000000000;
pt  = 64'h0000000000000000;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'hffffffffffffffff;
pt  = 64'hffffffffffffffff;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h3000000000000000;
pt  = 64'h1000000000000001;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h1111111111111111;
pt  = 64'h1111111111111111;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h0123456789abcdef;
pt  = 64'h1111111111111111;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h1111111111111111;
pt  = 64'h0123456789abcdef;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h0000000000000000;
pt  = 64'h0000000000000000;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'hfedcba9876543210;
pt  = 64'h0123456789abcdef;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h7ca110454a1a6e57;
pt  = 64'h01a1d6d039776742;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h0131d9619dc1376e;
pt  = 64'h5cd54ca83def57da;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h07a1133e4a0b2686;
pt  = 64'h0248d43806f67172;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h3849674c2602319e;
pt  = 64'h51454b582ddf440a;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h04b915ba43feb5b6;
pt  = 64'h42fd443059577fa2;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h0113b970fd34f2ce;
pt  = 64'h059b5e0851cf143a;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h0170f175468fb5e6;
pt  = 64'h0756d8e0774761d2;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h43297fad38e373fe;
pt  = 64'h762514b829bf486a;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h07a7137045da2a16;
pt  = 64'h3bdd119049372802;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h04689104c2fd3b2f;
pt  = 64'h26955f6835af609a;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h37d06bb516cb7546;
pt  = 64'h164d5e404f275232;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h1f08260d1ac2465e;
pt  = 64'h6b056e18759f5cca;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h584023641aba6176;
pt  = 64'h004bd6ef09176062;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

key = 64'h025816164629b007;
pt  = 64'h480d39006ee762f2;
for(i=0;i<16;i=i+1) begin #1 clk=0; #1 clk=1; end

/*
int testkeys[]= // key, pt, ct
{
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x8ca64de9, 0xc1b123a7,
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x7359b216, 0x3e4edc58,
0x30000000, 0x00000000, 0x10000000, 0x00000001, 0x958e6e62, 0x7a05557b,
0x11111111, 0x11111111, 0x11111111, 0x11111111, 0xf40379ab, 0x9e0ec533,
0x01234567, 0x89abcdef, 0x11111111, 0x11111111, 0x17668dfc, 0x7292532d,
0x11111111, 0x11111111, 0x01234567, 0x89abcdef, 0x8a5ae1f8, 0x1ab8f2dd,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x8ca64de9, 0xc1b123a7,
0xfedcba98, 0x76543210, 0x01234567, 0x89abcdef, 0xed39d950, 0xfa74bcc4,
0x7ca11045, 0x4a1a6e57, 0x01a1d6d0, 0x39776742, 0x690f5b0d, 0x9a26939b,
0x0131d961, 0x9dc1376e, 0x5cd54ca8, 0x3def57da, 0x7a389d10, 0x354bd271,
0x07a1133e, 0x4a0b2686, 0x0248d438, 0x06f67172, 0x868ebb51, 0xcab4599a,
0x3849674c, 0x2602319e, 0x51454b58, 0x2ddf440a, 0x7178876e, 0x01f19b2a,
0x04b915ba, 0x43feb5b6, 0x42fd4430, 0x59577fa2, 0xaf37fb42, 0x1f8c4095,
0x0113b970, 0xfd34f2ce, 0x059b5e08, 0x51cf143a, 0x86a560f1, 0x0ec6d85b,
0x0170f175, 0x468fb5e6, 0x0756d8e0, 0x774761d2, 0x0cd3da02, 0x0021dc09,
0x43297fad, 0x38e373fe, 0x762514b8, 0x29bf486a, 0xea676b2c, 0xb7db2b7a,
0x07a71370, 0x45da2a16, 0x3bdd1190, 0x49372802, 0xdfd64a81, 0x5caf1a0f,
0x04689104, 0xc2fd3b2f, 0x26955f68, 0x35af609a, 0x5c513c9c, 0x4886c088,
0x37d06bb5, 0x16cb7546, 0x164d5e40, 0x4f275232, 0x0a2aeeae, 0x3ff4ab77,
0x1f08260d, 0x1ac2465e, 0x6b056e18, 0x759f5cca, 0xef1bf03e, 0x5dfa575a,
0x58402364, 0x1aba6176, 0x004bd6ef, 0x09176062, 0x88bf0db6, 0xd70dee56,
0x02581616, 0x4629b007, 0x480d3900, 0x6ee762f2, 0xa1f99155, 0x41020b56,
0x49793ebc, 0x79b3258f, 0x437540c8, 0x698f3cfa, 0x6fbf1caf, 0xcffd0556,
0x4fb05e15, 0x15ab73a7, 0x072d43a0, 0x77075292, 0x2f22e49b, 0xab7ca1ac,
0x49e95d6d, 0x4ca229bf, 0x02fe5577, 0x8117f12a, 0x5a6b612c, 0xc26cce4a,
0x018310dc, 0x409b26d6, 0x1d9d5c50, 0x18f728c2, 0x5f4c038e, 0xd12b2e41,
0x1c587f1c, 0x13924fef, 0x30553228, 0x6d6f295a, 0x63fac0d0, 0x34d9f793,
0x01010101, 0x01010101, 0x01234567, 0x89abcdef, 0x617b3a0c, 0xe8f07100,
0x1f1f1f1f, 0x0e0e0e0e, 0x01234567, 0x89abcdef, 0xdb958605, 0xf8c8c606,
0xe0fee0fe, 0xf1fef1fe, 0x01234567, 0x89abcdef, 0xedbfd1c6, 0x6c29ccc7,
0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x355550b2, 0x150e2451,
0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0xcaaaaf4d, 0xeaf1dbae,
0x01234567, 0x89abcdef, 0x00000000, 0x00000000, 0xd5d44ff7, 0x20683d0d,
0xfedcba98, 0x76543210, 0xffffffff, 0xffffffff, 0x2a2bb008, 0xdf97c2f2,
};
*/

end

endmodule

module des(pt, key, ct, clk);
input 	[1:64] pt;
input 	[1:64] key;
output 	[1:64] ct;
input 	clk;
wire 	[1:48] k1x,k2x,k3x,k4x,k5x,k6x,k7x,k8x,k9x,k10x,k11x,k12x,k13x,k14x,k15x,k16x;
wire	[1:32] l0x,l1x,l2x,l3x,l4x,l5x,l6x,l7x,l8x,l9x,l10x,l11x,l12x,l13x,l14x,l15x,l16x;
wire	[1:32] r0x,r1x,r2x,r3x,r4x,r5x,r6x,r7x,r8x,r9x,r10x,r11x,r12x,r13x,r14x,r15x,r16x;

keysched keysched(key, k1x,k2x,k3x,k4x,k5x,k6x,k7x,k8x,k9x,k10x,k11x,k12x,k13x,k14x,k15x,k16x);
ip ip(pt, l0x, r0x);
roundfunc round1(clk, l0x, r0x, l1x, r1x, k1x);
roundfunc round2(clk, l1x, r1x, l2x, r2x, k2x);
roundfunc round3(clk, l2x, r2x, l3x, r3x, k3x);
roundfunc round4(clk, l3x, r3x, l4x, r4x, k4x);
roundfunc round5(clk, l4x, r4x, l5x, r5x, k5x);
roundfunc round6(clk, l5x, r5x, l6x, r6x, k6x);
roundfunc round7(clk, l6x, r6x, l7x, r7x, k7x);
roundfunc round8(clk, l7x, r7x, l8x, r8x, k8x);
roundfunc round9(clk, l8x, r8x, l9x, r9x, k9x);
roundfunc round10(clk, l9x, r9x, l10x, r10x, k10x);
roundfunc round11(clk, l10x, r10x, l11x, r11x, k11x);
roundfunc round12(clk, l11x, r11x, l12x, r12x, k12x);
roundfunc round13(clk, l12x, r12x, l13x, r13x, k13x);
roundfunc round14(clk, l13x, r13x, l14x, r14x, k14x);
roundfunc round15(clk, l14x, r14x, l15x, r15x, k15x);
roundfunc round16(clk, l15x, r15x, l16x, r16x, k16x);
fp fp(r16x, l16x, ct);

endmodule


module pc1(key, c0x, d0x);
input 	[1:64] key;
output 	[1:28] c0x, d0x;
wire 	[1:56] XX;

assign XX[1]=key[57]; assign  XX[2]=key[49]; assign  XX[3]=key[41]; assign  XX[4]=key[33]; assign  XX[5]=key[25]; assign  XX[6]=key[17]; assign  XX[7]=key[9];
assign XX[8]=key[1]; assign  XX[9]=key[58]; assign  XX[10]=key[50]; assign XX[11]=key[42]; assign XX[12]=key[34]; assign XX[13]=key[26]; assign XX[14]=key[18];
assign XX[15]=key[10]; assign XX[16]=key[2]; assign  XX[17]=key[59]; assign XX[18]=key[51]; assign XX[19]=key[43]; assign XX[20]=key[35]; assign XX[21]=key[27];
assign XX[22]=key[19]; assign XX[23]=key[11]; assign XX[24]=key[3]; assign  XX[25]=key[60]; assign XX[26]=key[52]; assign XX[27]=key[44]; assign XX[28]=key[36];
assign XX[29]=key[63]; assign XX[30]=key[55]; assign XX[31]=key[47]; assign XX[32]=key[39]; assign XX[33]=key[31]; assign XX[34]=key[23]; assign XX[35]=key[15];
assign XX[36]=key[7]; assign  XX[37]=key[62]; assign XX[38]=key[54]; assign XX[39]=key[46]; assign XX[40]=key[38]; assign XX[41]=key[30]; assign XX[42]=key[22];
assign XX[43]=key[14]; assign XX[44]=key[6]; assign  XX[45]=key[61]; assign XX[46]=key[53]; assign XX[47]=key[45]; assign XX[48]=key[37]; assign XX[49]=key[29];
assign XX[50]=key[21]; assign XX[51]=key[13]; assign XX[52]=key[5]; assign  XX[53]=key[28]; assign XX[54]=key[20]; assign XX[55]=key[12]; assign XX[56]=key[4];

assign c0x=XX[1:28]; assign d0x=XX[29:56];

endmodule


module pc2(c,d,k);
input 	[1:28] c,d;
output 	[1:48] k;
wire 	[1:56] YY;

        assign YY[1:28]=c;         assign YY[29:56]=d;

        assign k[1]=YY[14];   assign k[2]=YY[17];   assign k[3]=YY[11];   assign k[4]=YY[24];   assign k[5]=YY[1];    assign k[6]=YY[5];
        assign k[7]=YY[3];    assign k[8]=YY[28];   assign k[9]=YY[15];   assign k[10]=YY[6];   assign k[11]=YY[21];  assign k[12]=YY[10];
        assign k[13]=YY[23];  assign k[14]=YY[19];  assign k[15]=YY[12];  assign k[16]=YY[4];   assign k[17]=YY[26];  assign k[18]=YY[8];
        assign k[19]=YY[16];  assign k[20]=YY[7];   assign k[21]=YY[27];  assign k[22]=YY[20];  assign k[23]=YY[13];  assign k[24]=YY[2];
        assign k[25]=YY[41];  assign k[26]=YY[52];  assign k[27]=YY[31];  assign k[28]=YY[37];  assign k[29]=YY[47];  assign k[30]=YY[55];
        assign k[31]=YY[30];  assign k[32]=YY[40];  assign k[33]=YY[51];  assign k[34]=YY[45];  assign k[35]=YY[33];  assign k[36]=YY[48];
        assign k[37]=YY[44];  assign k[38]=YY[49];  assign k[39]=YY[39];  assign k[40]=YY[56];  assign k[41]=YY[34];  assign k[42]=YY[53];
        assign k[43]=YY[46];  assign k[44]=YY[42];  assign k[45]=YY[50];  assign k[46]=YY[36];  assign k[47]=YY[29];  assign k[48]=YY[32];
endmodule


module rol1(o, i);
output 	[1:28] o;
input	[1:28] i;

assign o={i[2:28],i[1]};

endmodule


module rol2(o, i);
output 	[1:28] o;
input	[1:28] i;

assign o={i[3:28],i[1:2]};
endmodule


module keysched(key,k1x,k2x,k3x,k4x,k5x,k6x,k7x,k8x,k9x,k10x,k11x,k12x,k13x,k14x,k15x,k16x);
input 	[1:64] key;
output 	[1:48] k1x,k2x,k3x,k4x,k5x,k6x,k7x,k8x,k9x,k10x,k11x,k12x,k13x,k14x,k15x,k16x;
wire 	[1:28] c0x,c1x,c2x,c3x,c4x,c5x,c6x,c7x,c8x,c9x,c10x,c11x,c12x,c13x,c14x,c15x,c16x;
wire 	[1:28] d0x,d1x,d2x,d3x,d4x,d5x,d6x,d7x,d8x,d9x,d10x,d11x,d12x,d13x,d14x,d15x,d16x;

pc1 pc1(key, c0x, d0x);

rol1 rc1(c1x, c0x); rol1 rd1(d1x, d0x);
rol1 rc2(c2x, c1x); rol1 rd2(d2x, d1x);
rol2 rc3(c3x, c2x); rol2 rd3(d3x, d2x);
rol2 rc4(c4x, c3x); rol2 rd4(d4x, d3x);
rol2 rc5(c5x, c4x); rol2 rd5(d5x, d4x);
rol2 rc6(c6x, c5x); rol2 rd6(d6x, d5x);
rol2 rc7(c7x, c6x); rol2 rd7(d7x, d6x);
rol2 rc8(c8x, c7x); rol2 rd8(d8x, d7x);
rol1 rc9(c9x, c8x); rol1 rd9(d9x, d8x);
rol2 rca(c10x, c9x); rol2 rda(d10x, d9x);
rol2 rcb(c11x, c10x); rol2 rdb(d11x, d10x);
rol2 rcc(c12x, c11x); rol2 rdc(d12x, d11x);
rol2 rcd(c13x, c12x); rol2 rdd(d13x, d12x);
rol2 rce(c14x, c13x); rol2 rde(d14x, d13x);
rol2 rcf(c15x, c14x); rol2 rdf(d15x, d14x);
rol1 rcg(c16x, c15x); rol1 rdg(d16x, d15x);


pc2 pc2x1(c1x,d1x,k1x);
pc2 pc2x2(c2x,d2x,k2x);
pc2 pc2x3(c3x,d3x,k3x);
pc2 pc2x4(c4x,d4x,k4x);
pc2 pc2x5(c5x,d5x,k5x);
pc2 pc2x6(c6x,d6x,k6x);
pc2 pc2x7(c7x,d7x,k7x);
pc2 pc2x8(c8x,d8x,k8x);
pc2 pc2x9(c9x,d9x,k9x);
pc2 pc2x10(c10x,d10x,k10x);
pc2 pc2x11(c11x,d11x,k11x);
pc2 pc2x12(c12x,d12x,k12x);
pc2 pc2x13(c13x,d13x,k13x);
pc2 pc2x14(c14x,d14x,k14x);
pc2 pc2x15(c15x,d15x,k15x);
pc2 pc2x16(c16x,d16x,k16x);

endmodule


module s1(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
				6'b000000 :	so=4'he;
				6'b000010 :	so=4'h4;
				6'b000100 :	so=4'hd;
				6'b000110 :	so=4'h1;
				6'b001000 :	so=4'h2;
				6'b001010 :	so=4'hf;
				6'b001100 :	so=4'hb;
				6'b001110 :	so=4'h8;
				6'b010000 :	so=4'h3;
				6'b010010 :	so=4'ha;
				6'b010100 :	so=4'h6;
				6'b010110 :	so=4'hc;
				6'b011000 :	so=4'h5;
				6'b011010 :	so=4'h9;
				6'b011100 :	so=4'h0;
				6'b011110 :	so=4'h7;
				6'b000001 :	so=4'h0;
				6'b000011 :	so=4'hf;
				6'b000101 :	so=4'h7;
				6'b000111 :	so=4'h4;
				6'b001001 :	so=4'he;
				6'b001011 :	so=4'h2;
				6'b001101 :	so=4'hd;
				6'b001111 :	so=4'h1;
				6'b010001 :	so=4'ha;
				6'b010011 :	so=4'h6;
				6'b010101 :	so=4'hc;
				6'b010111 :	so=4'hb;
				6'b011001 :	so=4'h9;
				6'b011011 :	so=4'h5;
				6'b011101 :	so=4'h3;
				6'b011111 :	so=4'h8;
				6'b100000 :	so=4'h4;
				6'b100010 :	so=4'h1;
				6'b100100 :	so=4'he;
				6'b100110 :	so=4'h8;
				6'b101000 :	so=4'hd;
				6'b101010 :	so=4'h6;
				6'b101100 :	so=4'h2;
				6'b101110 :	so=4'hb;
				6'b110000 :	so=4'hf;
				6'b110010 :	so=4'hc;
				6'b110100 :	so=4'h9;
				6'b110110 :	so=4'h7;
				6'b111000 :	so=4'h3;
				6'b111010 :	so=4'ha;
				6'b111100 :	so=4'h5;
				6'b111110 :	so=4'h0;
				6'b100001 :	so=4'hf;
				6'b100011 :	so=4'hc;
				6'b100101 :	so=4'h8;
				6'b100111 :	so=4'h2;
				6'b101001 :	so=4'h4;
				6'b101011 :	so=4'h9;
				6'b101101 :	so=4'h1;
				6'b101111 :	so=4'h7;
				6'b110001 :	so=4'h5;
				6'b110011 :	so=4'hb;
				6'b110101 :	so=4'h3;
				6'b110111 :	so=4'he;
				6'b111001 :	so=4'ha;
				6'b111011 :	so=4'h0;
				6'b111101 :	so=4'h6;
				default         so=4'hd;
			endcase
endmodule


module s2(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'hf;
                                6'b000010 :        so=4'h1;
                                6'b000100 :        so=4'h8;
                                6'b000110 :        so=4'he;
                                6'b001000 :        so=4'h6;
                                6'b001010 :        so=4'hb;
                                6'b001100 :        so=4'h3;
                                6'b001110 :        so=4'h4;
                                6'b010000 :        so=4'h9;
                                6'b010010 :        so=4'h7;
                                6'b010100 :        so=4'h2;
                                6'b010110 :        so=4'hd;
                                6'b011000 :        so=4'hc;
                                6'b011010 :        so=4'h0;
                                6'b011100 :        so=4'h5;
                                6'b011110 :        so=4'ha;
                                6'b000001 :        so=4'h3;
                                6'b000011 :        so=4'hd;
                                6'b000101 :        so=4'h4;
                                6'b000111 :        so=4'h7;
                                6'b001001 :        so=4'hf;
                                6'b001011 :        so=4'h2;
                                6'b001101 :        so=4'h8;
                                6'b001111 :        so=4'he;
                                6'b010001 :        so=4'hc;
                                6'b010011 :        so=4'h0;
                                6'b010101 :        so=4'h1;
                                6'b010111 :        so=4'ha;
                                6'b011001 :        so=4'h6;
                                6'b011011 :        so=4'h9;
                                6'b011101 :        so=4'hb;
                                6'b011111 :        so=4'h5;
                                6'b100000 :        so=4'h0;
                                6'b100010 :        so=4'he;
                                6'b100100 :        so=4'h7;
                                6'b100110 :        so=4'hb;
                                6'b101000 :        so=4'ha;
                                6'b101010 :        so=4'h4;
                                6'b101100 :        so=4'hd;
                                6'b101110 :        so=4'h1;
                                6'b110000 :        so=4'h5;
                                6'b110010 :        so=4'h8;
                                6'b110100 :        so=4'hc;
                                6'b110110 :        so=4'h6;
                                6'b111000 :        so=4'h9;
                                6'b111010 :        so=4'h3;
                                6'b111100 :        so=4'h2;
                                6'b111110 :        so=4'hf;
                                6'b100001 :        so=4'hd;
                                6'b100011 :        so=4'h8;
                                6'b100101 :        so=4'ha;
                                6'b100111 :        so=4'h1;
                                6'b101001 :        so=4'h3;
                                6'b101011 :        so=4'hf;
                                6'b101101 :        so=4'h4;
                                6'b101111 :        so=4'h2;
                                6'b110001 :        so=4'hb;
                                6'b110011 :        so=4'h6;
                                6'b110101 :        so=4'h7;
                                6'b110111 :        so=4'hc;
                                6'b111001 :        so=4'h0;
                                6'b111011 :        so=4'h5;
                                6'b111101 :        so=4'he;
                                default            so=4'h9;
			endcase
endmodule


module s3(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'ha;
                                6'b000010 :        so=4'h0;
                                6'b000100 :        so=4'h9;
                                6'b000110 :        so=4'he;
                                6'b001000 :        so=4'h6;
                                6'b001010 :        so=4'h3;
                                6'b001100 :        so=4'hf;
                                6'b001110 :        so=4'h5;
                                6'b010000 :        so=4'h1;
                                6'b010010 :        so=4'hd;
                                6'b010100 :        so=4'hc;
                                6'b010110 :        so=4'h7;
                                6'b011000 :        so=4'hb;
                                6'b011010 :        so=4'h4;
                                6'b011100 :        so=4'h2;
                                6'b011110 :        so=4'h8;
                                6'b000001 :        so=4'hd;
                                6'b000011 :        so=4'h7;
                                6'b000101 :        so=4'h0;
                                6'b000111 :        so=4'h9;
                                6'b001001 :        so=4'h3;
                                6'b001011 :        so=4'h4;
                                6'b001101 :        so=4'h6;
                                6'b001111 :        so=4'ha;
                                6'b010001 :        so=4'h2;
                                6'b010011 :        so=4'h8;
                                6'b010101 :        so=4'h5;
                                6'b010111 :        so=4'he;
                                6'b011001 :        so=4'hc;
                                6'b011011 :        so=4'hb;
                                6'b011101 :        so=4'hf;
                                6'b011111 :        so=4'h1;
                                6'b100000 :        so=4'hd;
                                6'b100010 :        so=4'h6;
                                6'b100100 :        so=4'h4;
                                6'b100110 :        so=4'h9;
                                6'b101000 :        so=4'h8;
                                6'b101010 :        so=4'hf;
                                6'b101100 :        so=4'h3;
                                6'b101110 :        so=4'h0;
                                6'b110000 :        so=4'hb;
                                6'b110010 :        so=4'h1;
                                6'b110100 :        so=4'h2;
                                6'b110110 :        so=4'hc;
                                6'b111000 :        so=4'h5;
                                6'b111010 :        so=4'ha;
                                6'b111100 :        so=4'he;
                                6'b111110 :        so=4'h7;
                                6'b100001 :        so=4'h1;
                                6'b100011 :        so=4'ha;
                                6'b100101 :        so=4'hd;
                                6'b100111 :        so=4'h0;
                                6'b101001 :        so=4'h6;
                                6'b101011 :        so=4'h9;
                                6'b101101 :        so=4'h8;
                                6'b101111 :        so=4'h7;
                                6'b110001 :        so=4'h4;
                                6'b110011 :        so=4'hf;
                                6'b110101 :        so=4'he;
                                6'b110111 :        so=4'h3;
                                6'b111001 :        so=4'hb;
                                6'b111011 :        so=4'h5;
                                6'b111101 :        so=4'h2;
                                default            so=4'hc;
			endcase
endmodule


module s4(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'h7;
                                6'b000010 :        so=4'hd;
                                6'b000100 :        so=4'he;
                                6'b000110 :        so=4'h3;
                                6'b001000 :        so=4'h0;
                                6'b001010 :        so=4'h6;
                                6'b001100 :        so=4'h9;
                                6'b001110 :        so=4'ha;
                                6'b010000 :        so=4'h1;
                                6'b010010 :        so=4'h2;
                                6'b010100 :        so=4'h8;
                                6'b010110 :        so=4'h5;
                                6'b011000 :        so=4'hb;
                                6'b011010 :        so=4'hc;
                                6'b011100 :        so=4'h4;
                                6'b011110 :        so=4'hf;
                                6'b000001 :        so=4'hd;
                                6'b000011 :        so=4'h8;
                                6'b000101 :        so=4'hb;
                                6'b000111 :        so=4'h5;
                                6'b001001 :        so=4'h6;
                                6'b001011 :        so=4'hf;
                                6'b001101 :        so=4'h0;
                                6'b001111 :        so=4'h3;
                                6'b010001 :        so=4'h4;
                                6'b010011 :        so=4'h7;
                                6'b010101 :        so=4'h2;
                                6'b010111 :        so=4'hc;
                                6'b011001 :        so=4'h1;
                                6'b011011 :        so=4'ha;
                                6'b011101 :        so=4'he;
                                6'b011111 :        so=4'h9;
                                6'b100000 :        so=4'ha;
                                6'b100010 :        so=4'h6;
                                6'b100100 :        so=4'h9;
                                6'b100110 :        so=4'h0;
                                6'b101000 :        so=4'hc;
                                6'b101010 :        so=4'hb;
                                6'b101100 :        so=4'h7;
                                6'b101110 :        so=4'hd;
                                6'b110000 :        so=4'hf;
                                6'b110010 :        so=4'h1;
                                6'b110100 :        so=4'h3;
                                6'b110110 :        so=4'he;
                                6'b111000 :        so=4'h5;
                                6'b111010 :        so=4'h2;
                                6'b111100 :        so=4'h8;
                                6'b111110 :        so=4'h4;
                                6'b100001 :        so=4'h3;
                                6'b100011 :        so=4'hf;
                                6'b100101 :        so=4'h0;
                                6'b100111 :        so=4'h6;
                                6'b101001 :        so=4'ha;
                                6'b101011 :        so=4'h1;
                                6'b101101 :        so=4'hd;
                                6'b101111 :        so=4'h8;
                                6'b110001 :        so=4'h9;
                                6'b110011 :        so=4'h4;
                                6'b110101 :        so=4'h5;
                                6'b110111 :        so=4'hb;
                                6'b111001 :        so=4'hc;
                                6'b111011 :        so=4'h7;
                                6'b111101 :        so=4'h2;
                                default            so=4'he;
			endcase
endmodule


module s5(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'h2;
                                6'b000010 :        so=4'hc;
                                6'b000100 :        so=4'h4;
                                6'b000110 :        so=4'h1;
                                6'b001000 :        so=4'h7;
                                6'b001010 :        so=4'ha;
                                6'b001100 :        so=4'hb;
                                6'b001110 :        so=4'h6;
                                6'b010000 :        so=4'h8;
                                6'b010010 :        so=4'h5;
                                6'b010100 :        so=4'h3;
                                6'b010110 :        so=4'hf;
                                6'b011000 :        so=4'hd;
                                6'b011010 :        so=4'h0;
                                6'b011100 :        so=4'he;
                                6'b011110 :        so=4'h9;
                                6'b000001 :        so=4'he;
                                6'b000011 :        so=4'hb;
                                6'b000101 :        so=4'h2;
                                6'b000111 :        so=4'hc;
                                6'b001001 :        so=4'h4;
                                6'b001011 :        so=4'h7;
                                6'b001101 :        so=4'hd;
                                6'b001111 :        so=4'h1;
                                6'b010001 :        so=4'h5;
                                6'b010011 :        so=4'h0;
                                6'b010101 :        so=4'hf;
                                6'b010111 :        so=4'ha;
                                6'b011001 :        so=4'h3;
                                6'b011011 :        so=4'h9;
                                6'b011101 :        so=4'h8;
                                6'b011111 :        so=4'h6;
                                6'b100000 :        so=4'h4;
                                6'b100010 :        so=4'h2;
                                6'b100100 :        so=4'h1;
                                6'b100110 :        so=4'hb;
                                6'b101000 :        so=4'ha;
                                6'b101010 :        so=4'hd;
                                6'b101100 :        so=4'h7;
                                6'b101110 :        so=4'h8;
                                6'b110000 :        so=4'hf;
                                6'b110010 :        so=4'h9;
                                6'b110100 :        so=4'hc;
                                6'b110110 :        so=4'h5;
                                6'b111000 :        so=4'h6;
                                6'b111010 :        so=4'h3;
                                6'b111100 :        so=4'h0;
                                6'b111110 :        so=4'he;
                                6'b100001 :        so=4'hb;
                                6'b100011 :        so=4'h8;
                                6'b100101 :        so=4'hc;
                                6'b100111 :        so=4'h7;
                                6'b101001 :        so=4'h1;
                                6'b101011 :        so=4'he;
                                6'b101101 :        so=4'h2;
                                6'b101111 :        so=4'hd;
                                6'b110001 :        so=4'h6;
                                6'b110011 :        so=4'hf;
                                6'b110101 :        so=4'h0;
                                6'b110111 :        so=4'h9;
                                6'b111001 :        so=4'ha;
                                6'b111011 :        so=4'h4;
                                6'b111101 :        so=4'h5;
                                default            so=4'h3;
			endcase
endmodule


module s6(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'hc;
                                6'b000010 :        so=4'h1;
                                6'b000100 :        so=4'ha;
                                6'b000110 :        so=4'hf;
                                6'b001000 :        so=4'h9;
                                6'b001010 :        so=4'h2;
                                6'b001100 :        so=4'h6;
                                6'b001110 :        so=4'h8;
                                6'b010000 :        so=4'h0;
                                6'b010010 :        so=4'hd;
                                6'b010100 :        so=4'h3;
                                6'b010110 :        so=4'h4;
                                6'b011000 :        so=4'he;
                                6'b011010 :        so=4'h7;
                                6'b011100 :        so=4'h5;
                                6'b011110 :        so=4'hb;
                                6'b000001 :        so=4'ha;
                                6'b000011 :        so=4'hf;
                                6'b000101 :        so=4'h4;
                                6'b000111 :        so=4'h2;
                                6'b001001 :        so=4'h7;
                                6'b001011 :        so=4'hc;
                                6'b001101 :        so=4'h9;
                                6'b001111 :        so=4'h5;
                                6'b010001 :        so=4'h6;
                                6'b010011 :        so=4'h1;
                                6'b010101 :        so=4'hd;
                                6'b010111 :        so=4'he;
                                6'b011001 :        so=4'h0;
                                6'b011011 :        so=4'hb;
                                6'b011101 :        so=4'h3;
                                6'b011111 :        so=4'h8;
                                6'b100000 :        so=4'h9;
                                6'b100010 :        so=4'he;
                                6'b100100 :        so=4'hf;
                                6'b100110 :        so=4'h5;
                                6'b101000 :        so=4'h2;
                                6'b101010 :        so=4'h8;
                                6'b101100 :        so=4'hc;
                                6'b101110 :        so=4'h3;
                                6'b110000 :        so=4'h7;
                                6'b110010 :        so=4'h0;
                                6'b110100 :        so=4'h4;
                                6'b110110 :        so=4'ha;
                                6'b111000 :        so=4'h1;
                                6'b111010 :        so=4'hd;
                                6'b111100 :        so=4'hb;
                                6'b111110 :        so=4'h6;
                                6'b100001 :        so=4'h4;
                                6'b100011 :        so=4'h3;
                                6'b100101 :        so=4'h2;
                                6'b100111 :        so=4'hc;
                                6'b101001 :        so=4'h9;
                                6'b101011 :        so=4'h5;
                                6'b101101 :        so=4'hf;
                                6'b101111 :        so=4'ha;
                                6'b110001 :        so=4'hb;
                                6'b110011 :        so=4'he;
                                6'b110101 :        so=4'h1;
                                6'b110111 :        so=4'h7;
                                6'b111001 :        so=4'h6;
                                6'b111011 :        so=4'h0;
                                6'b111101 :        so=4'h8;
                                default            so=4'hd;
			endcase
endmodule


module s7(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'h4;
                                6'b000010 :        so=4'hb;
                                6'b000100 :        so=4'h2;
                                6'b000110 :        so=4'he;
                                6'b001000 :        so=4'hf;
                                6'b001010 :        so=4'h0;
                                6'b001100 :        so=4'h8;
                                6'b001110 :        so=4'hd;
                                6'b010000 :        so=4'h3;
                                6'b010010 :        so=4'hc;
                                6'b010100 :        so=4'h9;
                                6'b010110 :        so=4'h7;
                                6'b011000 :        so=4'h5;
                                6'b011010 :        so=4'ha;
                                6'b011100 :        so=4'h6;
                                6'b011110 :        so=4'h1;
                                6'b000001 :        so=4'hd;
                                6'b000011 :        so=4'h0;
                                6'b000101 :        so=4'hb;
                                6'b000111 :        so=4'h7;
                                6'b001001 :        so=4'h4;
                                6'b001011 :        so=4'h9;
                                6'b001101 :        so=4'h1;
                                6'b001111 :        so=4'ha;
                                6'b010001 :        so=4'he;
                                6'b010011 :        so=4'h3;
                                6'b010101 :        so=4'h5;
                                6'b010111 :        so=4'hc;
                                6'b011001 :        so=4'h2;
                                6'b011011 :        so=4'hf;
                                6'b011101 :        so=4'h8;
                                6'b011111 :        so=4'h6;
                                6'b100000 :        so=4'h1;
                                6'b100010 :        so=4'h4;
                                6'b100100 :        so=4'hb;
                                6'b100110 :        so=4'hd;
                                6'b101000 :        so=4'hc;
                                6'b101010 :        so=4'h3;
                                6'b101100 :        so=4'h7;
                                6'b101110 :        so=4'he;
                                6'b110000 :        so=4'ha;
                                6'b110010 :        so=4'hf;
                                6'b110100 :        so=4'h6;
                                6'b110110 :        so=4'h8;
                                6'b111000 :        so=4'h0;
                                6'b111010 :        so=4'h5;
                                6'b111100 :        so=4'h9;
                                6'b111110 :        so=4'h2;
                                6'b100001 :        so=4'h6;
                                6'b100011 :        so=4'hb;
                                6'b100101 :        so=4'hd;
                                6'b100111 :        so=4'h8;
                                6'b101001 :        so=4'h1;
                                6'b101011 :        so=4'h4;
                                6'b101101 :        so=4'ha;
                                6'b101111 :        so=4'h7;
                                6'b110001 :        so=4'h9;
                                6'b110011 :        so=4'h5;
                                6'b110101 :        so=4'h0;
                                6'b110111 :        so=4'hf;
                                6'b111001 :        so=4'he;
                                6'b111011 :        so=4'h2;
                                6'b111101 :        so=4'h3;
                                default            so=4'hc;
			endcase
endmodule


module s8(clk, b, so);
input 	clk;
input 	[1:6] b;
output 	[1:4] so;
reg 	[1:4] so;

	always @(posedge clk)
		casex(b)
                                6'b000000 :        so=4'hd;
                                6'b000010 :        so=4'h2;
                                6'b000100 :        so=4'h8;
                                6'b000110 :        so=4'h4;
                                6'b001000 :        so=4'h6;
                                6'b001010 :        so=4'hf;
                                6'b001100 :        so=4'hb;
                                6'b001110 :        so=4'h1;
                                6'b010000 :        so=4'ha;
                                6'b010010 :        so=4'h9;
                                6'b010100 :        so=4'h3;
                                6'b010110 :        so=4'he;
                                6'b011000 :        so=4'h5;
                                6'b011010 :        so=4'h0;
                                6'b011100 :        so=4'hc;
                                6'b011110 :        so=4'h7;
                                6'b000001 :        so=4'h1;
                                6'b000011 :        so=4'hf;
                                6'b000101 :        so=4'hd;
                                6'b000111 :        so=4'h8;
                                6'b001001 :        so=4'ha;
                                6'b001011 :        so=4'h3;
                                6'b001101 :        so=4'h7;
                                6'b001111 :        so=4'h4;
                                6'b010001 :        so=4'hc;
                                6'b010011 :        so=4'h5;
                                6'b010101 :        so=4'h6;
                                6'b010111 :        so=4'hb;
                                6'b011001 :        so=4'h0;
                                6'b011011 :        so=4'he;
                                6'b011101 :        so=4'h9;
                                6'b011111 :        so=4'h2;
                                6'b100000 :        so=4'h7;
                                6'b100010 :        so=4'hb;
                                6'b100100 :        so=4'h4;
                                6'b100110 :        so=4'h1;
                                6'b101000 :        so=4'h9;
                                6'b101010 :        so=4'hc;
                                6'b101100 :        so=4'he;
                                6'b101110 :        so=4'h2;
                                6'b110000 :        so=4'h0;
                                6'b110010 :        so=4'h6;
                                6'b110100 :        so=4'ha;
                                6'b110110 :        so=4'hd;
                                6'b111000 :        so=4'hf;
                                6'b111010 :        so=4'h3;
                                6'b111100 :        so=4'h5;
                                6'b111110 :        so=4'h8;
                                6'b100001 :        so=4'h2;
                                6'b100011 :        so=4'h1;
                                6'b100101 :        so=4'he;
                                6'b100111 :        so=4'h7;
                                6'b101001 :        so=4'h4;
                                6'b101011 :        so=4'ha;
                                6'b101101 :        so=4'h8;
                                6'b101111 :        so=4'hd;
                                6'b110001 :        so=4'hf;
                                6'b110011 :        so=4'hc;
                                6'b110101 :        so=4'h9;
                                6'b110111 :        so=4'h0;
                                6'b111001 :        so=4'h3;
                                6'b111011 :        so=4'h5;
                                6'b111101 :        so=4'h6;
                                default            so=4'hb;
			endcase
endmodule


module ip(pt, l0x, r0x);
input 	[1:64] pt;
output 	[1:32] l0x, r0x;

assign l0x[1]=pt[58];         assign l0x[2]=pt[50];         assign l0x[3]=pt[42];         assign l0x[4]=pt[34];
assign l0x[5]=pt[26];         assign l0x[6]=pt[18];         assign l0x[7]=pt[10];         assign l0x[8]=pt[2];
assign l0x[9]=pt[60];         assign l0x[10]=pt[52];        assign l0x[11]=pt[44];        assign l0x[12]=pt[36];
assign l0x[13]=pt[28];        assign l0x[14]=pt[20];        assign l0x[15]=pt[12];        assign l0x[16]=pt[4];
assign l0x[17]=pt[62];        assign l0x[18]=pt[54];        assign l0x[19]=pt[46];        assign l0x[20]=pt[38];
assign l0x[21]=pt[30];        assign l0x[22]=pt[22];        assign l0x[23]=pt[14];        assign l0x[24]=pt[6];
assign l0x[25]=pt[64];        assign l0x[26]=pt[56];        assign l0x[27]=pt[48];        assign l0x[28]=pt[40];
assign l0x[29]=pt[32];        assign l0x[30]=pt[24];        assign l0x[31]=pt[16];        assign l0x[32]=pt[8];

assign r0x[1]=pt[57];         assign r0x[2]=pt[49];         assign r0x[3]=pt[41];         assign r0x[4]=pt[33];
assign r0x[5]=pt[25];         assign r0x[6]=pt[17];         assign r0x[7]=pt[9];          assign r0x[8]=pt[1];
assign r0x[9]=pt[59];         assign r0x[10]=pt[51];        assign r0x[11]=pt[43];        assign r0x[12]=pt[35];
assign r0x[13]=pt[27];        assign r0x[14]=pt[19];        assign r0x[15]=pt[11];        assign r0x[16]=pt[3];
assign r0x[17]=pt[61];        assign r0x[18]=pt[53];        assign r0x[19]=pt[45];        assign r0x[20]=pt[37];
assign r0x[21]=pt[29];        assign r0x[22]=pt[21];        assign r0x[23]=pt[13];        assign r0x[24]=pt[5];
assign r0x[25]=pt[63];        assign r0x[26]=pt[55];        assign r0x[27]=pt[47];        assign r0x[28]=pt[39];
assign r0x[29]=pt[31];        assign r0x[30]=pt[23];        assign r0x[31]=pt[15];        assign r0x[32]=pt[7];

endmodule


module xp(ri, e);
input [1:32] ri;
output [1:48] e;

assign e[1]=ri[32];   assign e[2]=ri[1];    assign e[3]=ri[2];    assign e[4]=ri[3];    assign e[5]=ri[4];    assign e[6]=ri[5];    assign e[7]=ri[4];    assign e[8]=ri[5];
assign e[9]=ri[6];    assign e[10]=ri[7];   assign e[11]=ri[8];   assign e[12]=ri[9];   assign e[13]=ri[8];   assign e[14]=ri[9];   assign e[15]=ri[10];  assign e[16]=ri[11];
assign e[17]=ri[12];  assign e[18]=ri[13];  assign e[19]=ri[12];  assign e[20]=ri[13];  assign e[21]=ri[14];  assign e[22]=ri[15];  assign e[23]=ri[16];  assign e[24]=ri[17];
assign e[25]=ri[16];  assign e[26]=ri[17];  assign e[27]=ri[18];  assign e[28]=ri[19];  assign e[29]=ri[20];  assign e[30]=ri[21];  assign e[31]=ri[20];  assign e[32]=ri[21];
assign e[33]=ri[22];  assign e[34]=ri[23];  assign e[35]=ri[24];  assign e[36]=ri[25];  assign e[37]=ri[24];  assign e[38]=ri[25];  assign e[39]=ri[26];  assign e[40]=ri[27];
assign e[41]=ri[28];  assign e[42]=ri[29];  assign e[43]=ri[28];  assign e[44]=ri[29];  assign e[45]=ri[30];  assign e[46]=ri[31];  assign e[47]=ri[32];  assign e[48]=ri[1];

endmodule


module desxor1(e,b1x,b2x,b3x,b4x,b5x,b6x,b7x,b8x,k);
input 	[1:48] e;
output 	[1:6] b1x,b2x,b3x,b4x,b5x,b6x,b7x,b8x;
input 	[1:48] k;
wire 	[1:48] XX;

assign         XX = k ^ e;
assign        b1x = XX[1:6];
assign        b2x = XX[7:12];
assign        b3x = XX[13:18];
assign        b4x = XX[19:24];
assign        b5x = XX[25:30];
assign        b6x = XX[31:36];
assign        b7x = XX[37:42];
assign        b8x = XX[43:48];

endmodule


module pp(so1x,so2x,so3x,so4x,so5x,so6x,so7x,so8x,ppo);
input 	[1:4] so1x,so2x,so3x,so4x,so5x,so6x,so7x,so8x;
output 	[1:32] ppo;
wire 	[1:32] XX;

        assign XX[1:4]=so1x;       assign XX[5:8]=so2x;       assign XX[9:12]=so3x;      assign XX[13:16]=so4x;
        assign XX[17:20]=so5x;     assign XX[21:24]=so6x;     assign XX[25:28]=so7x;     assign XX[29:32]=so8x;

        assign ppo[1]=XX[16];         assign ppo[2]=XX[7];          assign ppo[3]=XX[20];         assign ppo[4]=XX[21];
        assign ppo[5]=XX[29];         assign ppo[6]=XX[12];         assign ppo[7]=XX[28];         assign ppo[8]=XX[17];
        assign ppo[9]=XX[1];          assign ppo[10]=XX[15];        assign ppo[11]=XX[23];        assign ppo[12]=XX[26];
        assign ppo[13]=XX[5];         assign ppo[14]=XX[18];        assign ppo[15]=XX[31];        assign ppo[16]=XX[10];
        assign ppo[17]=XX[2];         assign ppo[18]=XX[8];         assign ppo[19]=XX[24];        assign ppo[20]=XX[14];
        assign ppo[21]=XX[32];        assign ppo[22]=XX[27];        assign ppo[23]=XX[3];         assign ppo[24]=XX[9];
        assign ppo[25]=XX[19];        assign ppo[26]=XX[13];        assign ppo[27]=XX[30];        assign ppo[28]=XX[6];
        assign ppo[29]=XX[22];        assign ppo[30]=XX[11];        assign ppo[31]=XX[4];         assign ppo[32]=XX[25];

endmodule


module desxor2(d,l,q);
input 	[1:32] d,l;
output 	[1:32] q;

assign q = d ^ l;

endmodule


module roundfunc(clk, li, ri, lo, ro, k);
input	clk;
input	[1:32] li, ri;
input	[1:48] k;
output	[1:32] lo, ro;

wire 	[1:48] e;
wire	[1:6] b1x,b2x,b3x,b4x,b5x,b6x,b7x,b8x;
wire	[1:4] so1x,so2x,so3x,so4x,so5x,so6x,so7x,so8x;
wire 	[1:32] ppo;

xp xp(ri, e);
desxor1 desxor1(e, b1x, b2x, b3x, b4x, b5x, b6x, b7x, b8x, k);
s1 s1(clk, b1x, so1x);
s2 s2(clk, b2x, so2x);
s3 s3(clk, b3x, so3x);
s4 s4(clk, b4x, so4x);
s5 s5(clk, b5x, so5x);
s6 s6(clk, b6x, so6x);
s7 s7(clk, b7x, so7x);
s8 s8(clk, b8x, so8x);
pp pp(so1x,so2x,so3x,so4x,so5x,so6x,so7x,so8x, ppo);
desxor2 desxor2(ppo, li, ro);

assign lo=ri;

endmodule


module fp(l,r,ct);
input 	[1:32] l,r;
output	[1:64] ct;

	assign ct[1]=r[8];	assign ct[2]=l[8];	assign ct[3]=r[16];	assign ct[4]=l[16];	assign ct[5]=r[24];	assign ct[6]=l[24];	assign ct[7]=r[32];	assign ct[8]=l[32];
	assign ct[9]=r[7];	assign ct[10]=l[7];	assign ct[11]=r[15];	assign ct[12]=l[15];	assign ct[13]=r[23];	assign ct[14]=l[23];	assign ct[15]=r[31];	assign ct[16]=l[31];
	assign ct[17]=r[6];	assign ct[18]=l[6];	assign ct[19]=r[14];	assign ct[20]=l[14];	assign ct[21]=r[22];	assign ct[22]=l[22];	assign ct[23]=r[30];	assign ct[24]=l[30];
	assign ct[25]=r[5];	assign ct[26]=l[5];	assign ct[27]=r[13];	assign ct[28]=l[13];	assign ct[29]=r[21];	assign ct[30]=l[21];	assign ct[31]=r[29];	assign ct[32]=l[29];
	assign ct[33]=r[4];	assign ct[34]=l[4];	assign ct[35]=r[12];	assign ct[36]=l[12];	assign ct[37]=r[20];	assign ct[38]=l[20];	assign ct[39]=r[28];	assign ct[40]=l[28];
	assign ct[41]=r[3];	assign ct[42]=l[3];	assign ct[43]=r[11];	assign ct[44]=l[11];	assign ct[45]=r[19];	assign ct[46]=l[19];	assign ct[47]=r[27];	assign ct[48]=l[27];
	assign ct[49]=r[2];	assign ct[50]=l[2];	assign ct[51]=r[10];	assign ct[52]=l[10];	assign ct[53]=r[18];	assign ct[54]=l[18];	assign ct[55]=r[26];	assign ct[56]=l[26];
	assign ct[57]=r[1];	assign ct[58]=l[1];	assign ct[59]=r[9];	assign ct[60]=l[9];	assign ct[61]=r[17];	assign ct[62]=l[17];	assign ct[63]=r[25];	assign ct[64]=l[25];

endmodule


