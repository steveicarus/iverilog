// Released under GPL 2.0
// (C) 2002 Tom Verbeure

module main;

	initial begin
		$display("============================");
		$display(">|This is a test|");
		$display("*|%s|", "This is a test");
		$display("*|", "This is a test", "|");

		$display(">| 65|");
		$display("*|%d|", "A");
		$display(">|16706|");
		$display("*|%d|", "AB");
		$display(">| 4276803|");
		$display("*|%d|", "ABC");
		$display(">|1094861636|");
		$display("*|%d|", "ABCD");

		$display(">|01000001|");
		$display("*|%b|", "A");
		$display(">|01000001010000100100001101000100|");
		$display("*|%b|", "ABCD");
		$display(">|01000001010000100100001101000100010010000100100101001010010010110100110001001101010011100100111101010000010100010101001001010011|");
		$display("*|%b|", "ABCDHIJKLMNOPQRS");

		$display(">|41|");
		$display("*|%h|", "A");
		$display(">|41424344|");
		$display("*|%h|", "ABCD");
		$display(">|4142434448494a4b4c4d4e4f50515253|");
		$display("*|%h|", "ABCDHIJKLMNOPQRS");
	end
endmodule
