module adding;

reg signed [20:0] s3, s4;
reg signed [21:0] t1, t2;
reg clk;
initial begin
	clk=0;
	s3=+400000;
	s4=+200000;
	#10;
	clk=1;
	#10;
	$display("%d %d", t1, t2);
	clk=0;
	s3=-400000;
	s4=-200000;
	#10;
	clk=1;
	#10;
	$display("%d %d", t1, t2);
	$display("%s", (t1==t2) ? "PASSED" : "FAIL");
end

always @(posedge clk) begin
	t1 <= s3 + 2*s4;
	t2 <= s3 + s4*2;
end

endmodule
