`timescale 1ns/1ns

module test;

reg pass = 1;

reg [3 : 0] A = 4'hf;
wire [3 : 0] a_lls, a_lrs;

reg signed [3 : 0] B = 7;
wire signed [3 : 0] b_als, b_ars;

assign a_lls = A<<4;
assign a_lrs = A>>4;
assign b_als = B<<<4;
assign b_ars = B>>>4;

initial begin
	#1;
	if (a_lls !== 4'b0) begin
		$display("FAILED assigning logical left shift");
		pass = 0;
	end
	if (a_lrs !== 4'b0) begin
		$display("FAILED assigning logical right shift");
		pass = 0;
	end
	if (b_als !== 4'b0) begin
		$display("FAILED assigning arithmetic left shift");
		pass = 0;
	end
	if (b_ars !== 4'h0) begin
		$display("FAILED assigning arithmetic right shift (0)");
		pass = 0;
	end
	#1 B = -8;
	#1;
	if (b_ars !== 4'hf) begin
		$display("FAILED assigning arithmetic right shift (1)");
		pass = 0;
	end

	if (pass) $display("PASSED");
end

endmodule // test
