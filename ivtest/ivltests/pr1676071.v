`begin_keywords "1364-2005"
`timescale 1ns / 1ns

module gentest;

reg [7:0] a=0, b=0;
wire co;
wire [7:0] result;

adder work(a, b, 1'b0, result, co);

integer cc;
initial begin
	for (cc=0; cc<10; cc=cc+1) begin
		a=a+1;
		#10;
		$display("%d %d %d", a, b, result);
		b=result;
	end
	if (b==55) $display("PASSED");
	else $display("FAIL");
end

endmodule

module adder(a, b, ci, out, co);
parameter SIZE=8;
input [SIZE-1:0] a;
input [SIZE-1:0] b;
input ci;
output [SIZE-1:0] out;
output co;

wire [SIZE:0] c;
assign c[0] = ci;
assign co = c[SIZE];
`ifdef NOGENERATE
	add1 bit0(a[0], b[0], c[0], out[0], c[0+1]);
	add1 bit1(a[1], b[1], c[1], out[1], c[1+1]);
	add1 bit2(a[2], b[2], c[2], out[2], c[2+1]);
	add1 bit3(a[3], b[3], c[3], out[3], c[3+1]);
	add1 bit4(a[4], b[4], c[4], out[4], c[4+1]);
	add1 bit5(a[5], b[5], c[5], out[5], c[5+1]);
	add1 bit6(a[6], b[6], c[6], out[6], c[6+1]);
	add1 bit7(a[7], b[7], c[7], out[7], c[7+1]);
`else
genvar i;
generate for(i=0; i<SIZE; i=i+1) begin:addbit
	add1 bit(a[i], b[i], c[i], out[i], c[i+1]);
end endgenerate
`endif

endmodule

module add1(a, b, ci, sum, co);
	input a, b, ci;
	output sum, co;
	assign {co,sum} = a + b + ci;
endmodule
`end_keywords
