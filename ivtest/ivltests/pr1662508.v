// pr1662508.v

`timescale 1ns / 1ns

module ram(
	input clk,
	input we,
	input  [9:0] addr,
	input  [15:0] data,
	output [15:0] read_bus
);

reg [15:0] ram[31:0];
assign read_bus = ram[addr[3:0]];
always @(posedge clk) if (we)
	ram[addr[3:0]] <= data;
endmodule


module ram_test;

reg clk;
reg fail=0;
integer cc;
initial begin
	for (cc = 0; cc < 33; cc=cc+1) begin
		clk = 0;  #5;
		clk = 1;  #5;
	end
	if (fail) $display("FAIL");
	else $display("PASSED");
end

reg we=0;
reg [9:0] addr=0;
reg [15:0] data=0;

always @(posedge clk) begin
	addr <= cc;
	data <= cc*cc;
	we <= cc<16;
end

wire [15:0] read_bus;
ram ram(clk, we, addr, data, read_bus);

always @(negedge clk) if (~we) begin
	$display("%d %d", addr, read_bus);
	if (read_bus !== addr[3:0]*addr[3:0]) fail=1;
end

endmodule
