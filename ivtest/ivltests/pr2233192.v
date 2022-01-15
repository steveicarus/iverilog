module bug05_integerRem;

reg passed;

reg signed[31:0] reg0;
reg signed[31:0] reg1;
reg signed[31:0] rrem;
wire signed[31:0] dividend=reg0;
wire signed[31:0] divisor=reg1;
wire signed[31:0] remainder;

assign remainder= dividend%divisor;

initial begin
	passed = 1'b1;
	reg0=32'hffffffff;
	reg1=32'h0d1f0796;
	//BUG here: remainder==32'h06b26fdd, should be   32'hffffffff
	#1 if (remainder !== 32'hffffffff) begin
		$display("Failed: CA remainder, expected 32'hffffffff, got %h",
		         remainder);
		passed = 1'b0;
	end

        rrem = reg0 % reg1;
	#1 if (rrem !== 32'hffffffff) begin
		$display("Failed: remainder, expected 32'hffffffff, got %h",
		         rrem);
		passed = 1'b0;
	end

	if (passed) $display("PASSED");
end

endmodule
