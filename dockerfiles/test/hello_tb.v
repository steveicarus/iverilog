`timescale 1ns / 1ns
`include "hello.v"

module hello_tb;

reg A;
wire B;

hello uut(A, B);

initial begin

	$dumpfile("hello_tb.vcd");
	$dumpvars(0, hello_tb);

	A = 0;
	#20;

	A = 1;
	#20;

	A = 0;
	#20;

	$display("Hello World test complete");

end

endmodule
