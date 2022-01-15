module bug04_integerDiv;

reg passed;

reg signed[31:0] reg0;
reg signed[31:0] reg1;
reg signed[31:0] rquot;
wire signed[31:0] dividend=reg0;
wire signed[31:0] divisor=reg1;
wire signed[31:0] quotient;

assign quotient= dividend/divisor;

initial begin
	passed = 1'b1;
	reg0=32'h76c3625e;
	reg1=32'hffffffff;
	//BUG here: quotient==32'hxxxxxxxx, should be   32'h893c9da2
	#1 if (quotient !== 32'h893c9da2) begin
		$display("Failed: CA division, expected 32'h893c9da2, got %h",
		         quotient);
		passed = 1'b0;
	end

	rquot = reg0/reg1;
	if (rquot !== 32'h893c9da2) begin
		$display("Failed: division, expected 32'h893c9da2, got %h",
		         rquot);
		passed = 1'b0;
	end

	if (passed) $display("PASSED");
end

endmodule
