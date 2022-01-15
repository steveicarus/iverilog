`timescale 1 ns / 1 ns
module short(inout [7:0] p, input en);
	assign p = en ? 8'h55 : 8'hzz;
endmodule

module long(inout [15:0] p, input en);
	assign p = en ? 16'haaaa : 16'hzzzz;
endmodule

module main;
	wire [15:0] bus;
	reg l_en, s_en;
	integer fails=0;
	long l(.p(bus), .en(l_en));
	short s(.p(bus[7:0]), .en(s_en));
	initial begin
		// $dumpfile("tri.vcd");
		// $dumpvars(3,main);
		l_en = 0;
		s_en = 0;
		#10;
		l_en = 1;
		#10;
		$display("s.p = %4x", s.p);
		if (s.p !== 8'haa) fails=1;
		#10;
		l_en = 0;
		s_en = 1;
		#10;
		$display("l.p = %4x", l.p);
		if (l.p !== 16'hzz55) fails=2;
		#10;
		s_en = 0;
		#10;
		if (fails == 0) $display("PASSED");
		else $display("FAILED ",fails);
	end
endmodule
