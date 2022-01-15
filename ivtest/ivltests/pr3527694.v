`timescale 1ns / 1ps

module top;

reg	sys_clk;
reg	sys_reset;

integer cnt, lastcnt;

initial begin
	sys_clk = 1'b1;
	sys_reset = 1'b1;
	cnt = 0;
	lastcnt = 100;
	while(cnt < lastcnt) begin
		#5;
		sys_clk = 1'b0;
		#5;
		sys_clk = 1'b1;
		if(cnt > 10) begin
			sys_reset = 1'b0;
		end
		cnt = cnt + 1;
	end
	#20;
	$dumpflush;
	$finish(2);
	$stop(2);
end

test1 #(
.WIDTH1(18),
.WIDTH2(18)
) test1 (
	.sys_clk(sys_clk),
	.sys_rst(sys_reset)
);

test2 #(
	.WIDTH1(18),
	.WIDTH2(18)
) test2_top (
	.sys_clk(sys_clk),
	.sys_rst(sys_reset)
);

endmodule

module test1
#(
parameter WIDTH1 = 1,
parameter WIDTH2 = 2
) (
input sys_clk,
input sys_rst
);

wire [15:0] w1 = WIDTH1;
wire [15:0] w2 = WIDTH2;

generate
if(WIDTH1 != 1) begin: not1

test2 #(
	.WIDTH1(18),
	.WIDTH2(18)
) test2_0 (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst)
);

test2 #(
	.WIDTH1(18),
	.WIDTH2(18)
) test2_1 (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst)
);

end
endgenerate

initial begin
	#40;
	$finish(0);
end

endmodule

module test2
#(
parameter WIDTH1 = 3,
parameter WIDTH2 = 4
) (
input sys_clk,
input sys_rst
);

localparam big_width = (WIDTH1 >= WIDTH2) ? WIDTH1 : WIDTH2;

initial begin
	$display("%m big_width: %h", big_width);
end

wire [15:0] w1 = WIDTH1;
wire [15:0] w2 = WIDTH2;
wire [15:0] bigw = big_width;

wire [31:0] out_data_a, out_data_b;
test3 #(
	.WIDTH1(WIDTH1),
	.WIDTH2(WIDTH2)
) test3_0 (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst)
);

test3 test3_1 (
	.sys_clk(sys_clk),
	.sys_rst(sys_rst)
);
	defparam test3_1.WIDTH1 = WIDTH1;
	defparam test3_1.WIDTH2 = WIDTH2;

endmodule

module test3 (sys_clk, sys_rst);
input sys_clk;
input sys_rst;

parameter WIDTH1 = 0;
parameter WIDTH2 = 0;

localparam big_width = (WIDTH1 >= WIDTH2) ? WIDTH1 : WIDTH2;

wire [31:0] wide = big_width;
wire [31:0] width1 = WIDTH1;
wire [31:0] width2 = WIDTH2;
initial begin
	$strobe("%m wide: %h, width1:%h, width2:%h", wide, width1, width2);
end


endmodule
