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
		$display("============================ myReg8 = 65");
		myReg8 = 65;
		$display(">| 65|");
		$display("*|%d|", myReg8);
		$display("*|",myReg8,"|");


		$display("============================ myReg14 = -10");
		myReg14 = -10;
		$display(">|16374|");
		$display("*|%d|", myReg14);
		$display("*|",myReg14,"|");

		$display("============================ myReg14 = 65");
		myReg14 = 65;
		$display(">1|   65|");
		$display("*1|%d|", myReg14);
		$display(">2|65|");
		$display("*2|%0d|", myReg14);
		$display(">3|   65|");
		$display("*3|%10d|", myReg14);
		$display(">4|   65|");
		$display("*4|%08d|", myReg14);
		$display("*4|%8d|", myReg14);
		$display(">5| 65|");
		$display("*5|%03d|", myReg14);
		$display("*5|%3d|", myReg14);

		$display("============================ myReg14 = 1000");
		myReg14 = 1000;
		$display(">|1000|");
		$display("*|%03d|", myReg14);

		$finish(0);

		$display("*|",myReg14,"|");

		$display(">|0041|");
		$display("*|%h|", myReg14);
		$display(">|00000001000001|");
		$display("*|%b|", myReg14);
		$display(">|41|");
		$display("*|%0h|", myReg14);
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

		$display(">|fffffff6|");
		$display("*|%h|", myInt);

		$display("============================ myReg32 = -10");
		myReg32 = -10;
		$display(">|4294967286|");
		$display("*|%d|", myReg32);
		$display("*|",myReg32,"|");

		$display(">|fffffff6|");
		$display("*|%h|", myReg32);

		$display("============================ myInt = 65");
		myInt = 65;
		$display(">|         65|");
		$display("*|%d|", myInt);
		$display("*|",myInt,"|");
		$display("*|   A|");
		$display(">|%s|", myInt);
		$display("*|A|");
		$display(">|%0s|", myInt);

		$display("============================ myReg32 = 65");
		myReg32 = 65;
		$display(">|        65|");
		$display("*|%d|", myReg32);
		$display("*|",myReg32,"|");
		$display("*|   A|");
		$display(">|%s|", myReg32);
		$display("*|A|");
		$display(">|%0s|", myReg32);

		$display("*|   A|");
		$display(">|%s|", "   A");
		$display("*|   A|");
		$display(">|%0s|", "   A");

		$display("*|0|");
		$display(">|%0t|", $time);
		$display("*|                   0|");
		$display(">|%t|", $time);
	end
endmodule
