// pr1866215

module A (CH, CL, SH, SL);

output	[31:0]	SL;
output	[31:0]	CL;
output	[47:32]	SH;
output	[47:32]	CH;

B B0	(CH, CL, SH, SL);

   assign	SH = 'hff;
   assign	SL = 32'haaaaaaaa;
   assign	CH = 'hff;
   assign	CL = 32'h55555555;

endmodule

module B (CH, CL, SH, SL);

input	[37:32]	CH;
input	[31:0]	CL;
input	[38:32]	SH;
input	[31:0]	SL;

C C0	(CH, CL, SH, SL);

endmodule

module C (CH, CL, SH, SL);

input	[38:32]	CH;
input	[31:0]	CL;
input	[39:32]	SH;
input	[31:0]	SL;

   initial #1 $display("CH=%h, CL=%h, SH=%h, SL=%h", CH, CL, SH, SL);

endmodule
