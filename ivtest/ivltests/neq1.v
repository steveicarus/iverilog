module main;

reg [2:0] a;

wire e0 = a==3'h0; wire n0 = a!=3'h0;
wire e1 = a==3'h1; wire n1 = a!=3'h1;
wire e2 = a==3'h2; wire n2 = a!=3'h2;
wire e3 = a==3'h3; wire n3 = a!=3'h3;
wire e4 = a==3'h4; wire n4 = a!=3'h4;
wire e5 = a==3'h5; wire n5 = a!=3'h5;
wire e6 = a==3'h6; wire n6 = a!=3'h6;
wire e7 = a==3'h7; wire n7 = a!=3'h7;
initial begin
	for (a=0; a<7; a=a+1) begin
		#1;
		$display("a=",a);
		$display(" 0 %d %d", e0, n0);
		$display(" 1 %d %d", e1, n1);
		$display(" 2 %d %d", e2, n2);
		$display(" 3 %d %d", e3, n3);
		$display(" 4 %d %d", e4, n4);
		$display(" 5 %d %d", e5, n5);
		$display(" 6 %d %d", e6, n6);
		$display(" 7 %d %d", e7, n7);
	end
end

endmodule
