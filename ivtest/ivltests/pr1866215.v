// pr1866215

module A (CH, CL, SH, SL);

wire	[31:6]	S1L;
wire	[39:32]	S1H;
wire	[31:6]	C1L;
wire	[38:32]	C1H;

output	[31:0]	SL;
output	[31:0]	CL;
output	[47:32]	SH;
output	[47:32]	CH;

B B0	(C1H[38:32], {C1L[31:6], CL[5:0]}, S1H[39:32], {S1L[31:6], SL[5:0]});

initial begin
   #1 $display("C1H=%h, {C1L, CL}={%h, %h}, S1H=%h, {S1L, SL}={%h, %h}",
	       C1H, C1L, CL, S1H, S1L, SL);
end

endmodule

module B (CH, CL, SH, SL);

output	[37:32]	CH;
output	[31:0]	CL;
output	[38:32]	SH;
output	[31:0]	SL;

C C0	(CH, CL, SH, SL);

endmodule

module C (CH, CL, SH, SL);

output	[38:32]	CH;
output	[31:0]	CL;
output	[39:32]	SH;
output	[31:0]	SL;

   assign	CH = 6'h33;
   assign	CL = 32'h55555555;
   assign	SH = 7'h66;
   assign	SL = 32'haaaaaaaa;
endmodule
