module t();

wire [63:0] a;
wire [63:0] b;

assign a = 3;

am dut(a, b);

initial
begin
	#1;
	if(b !== 2)
		$display("FAILED");
	else
		$display("PASSED");
	$finish;
end
endmodule

module am(
	a, b
);

input [15:0] a;
output [15:0] b;

assign b = a ^ 1;

endmodule
