// Released under GPL2.0
// (c) 2002 Tom Verbeure

module main;

	integer myInt;
	reg [39:0] myReg40;
	reg [0:39] myReg40r;
	reg [0:38] myReg39r;
	reg [13:0] myReg14;
	reg [7:0] myReg8;
	reg [31:0] myReg32;

	initial begin
		$display("============================ myReg14 = -10");
		myReg14 = -10;

		$display(">|16374|");
		$display("*|%d|", myReg14);
		$display("*|%0d|", myReg14);
		$display("*|",myReg14,"|");

		$display("============================ myReg14 = 65");
		myReg14 = 65;

		$display(">|   65|");
		$display("*|%d|", myReg14);
		$display("*|",myReg14,"|");
		$display(">|65|");
		$display("*|%0d|", myReg14);

		$display(">|0041|");
		$display("*|%h|", myReg14);
		$display(">|41|");
		$display("*|%0h|", myReg14);

		$display(">|00101|");
		$display("*|%o|", myReg14);
		$display(">|101|");
		$display("*|%0o|", myReg14);

		$display(">|00000001000001|");
		$display("*|%b|", myReg14);
		$display(">|1000001|");
		$display("*|%0b|", myReg14);

		$display(">| A|");
		$display("*|%s|", myReg14);
		$display(">|A|");
		$display("*|%0s|", myReg14);

		$display("============================ myInt = -10");
		myInt = -10;
		$display(">|        -10|");
		$display("*|%d|", myInt);
		$display("*|",myInt,"|");
		$display(">|-10|");
		$display("*|%0d|", myInt);

		$display(">|fffffff6|");
		$display("*|%h|", myInt);
		$display("*|%0h|", myInt);

		$display(">|37777777766|");
		$display("*|%o|", myInt);
		$display("*|%0o|", myInt);

		$display(">|11111111111111111111111111110110|");
		$display("*|%b|", myInt);
		$display("*|%0b|", myInt);

		$display("============================ myReg32 = -10");
		myReg32 = -10;
		$display(">|4294967286|");
		$display("*|%d|", myReg32);
		$display("*|%0d|", myReg32);
		$display("*|",myReg32,"|");

		$display(">|fffffff6|");
		$display("*|%h|", myReg32);
		$display("*|%0h|", myReg32);

		$display(">|37777777766|");
		$display("*|%o|", myReg32);
		$display("*|%0o|", myReg32);

		$display("============================ myInt = 65");
		myInt = 65;
		$display(">|         65|");
		$display("*|%d|", myInt);
		$display("*|",myInt,"|");
		$display(">|65|");
		$display("*|%0d|", myInt);

		$display(">|00000041|");
		$display("*|%h|", myInt);
		$display(">|41|");
		$display("*|%0h|", myInt);

		$display(">|00000000101|");
		$display("*|%o|", myInt);
		$display(">|101|");
		$display("*|%0o|", myInt);

		$display(">|00000000000000000000000001000001|");
		$display("*|%b|", myInt);
		$display(">|1000001|");
		$display("*|%0b|", myInt);

		$display("*|   A|");
		$display(">|%s|", myInt);
		$display("*|A|");
		$display(">|%0s|", myInt);

		$display("============================ Print \"   A\"");
		$display("*|   A|");
		$display(">|%s|", "   A");
		$display(">|%0s|", "   A");

		$display("============================ Print $time");
		$display("*|                   0|");
		$display(">|%t|", $time);
		$display("*|0|");
		$display(">|%0t|", $time);

	end
endmodule
