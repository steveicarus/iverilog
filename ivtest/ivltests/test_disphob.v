// Released under GPL2.0
// (c) 2002 Tom Verbeure

module main;

	integer myInt;
	reg [13:0] myReg14;
	reg [7:0] myReg8;
	reg [31:0] myReg32;

	initial begin
		$display("============================ myReg14 = 65");
		myReg14 = 65;

		$display(">|   65|");
		$display("*|",myReg14,"|");
		$write("*|",myReg14,"|\n");

		$display(">|0041|");
		$displayh("*|",myReg14,"|");
		$writeh("*|",myReg14,"|\n");

		$display(">|00101|");
		$displayo("*|",myReg14,"|");
		$writeo("*|",myReg14,"|\n");

		$display(">|00000001000001|");
		$displayb("*|",myReg14,"|");
		$writeb("*|",myReg14,"|\n");

		$display("============================ myInt = -10");
		myInt = -10;
		$display(">|        -10|");
		$display("*|",myInt,"|");

		$display(">|fffffff6|");
		$displayh("*|",myInt,"|");

		$display(">|37777777766|");
		$displayo("*|",myInt,"|");

		$display(">|11111111111111111111111111110110|");
		$displayb("*|",myInt,"|");

		$display("============================ myReg32 = -10");
		myReg32 = -10;
		$display(">|4294967286|");
		$display("*|",myReg32,"|");

		$display(">|fffffff6|");
		$displayh("*|",myReg32,"|");

		$display(">|37777777766|");
		$displayo("*|",myReg32,"|");

		$display(">|11111111111111111111111111110110|");
		$displayb("*|",myReg32,"|");

		$display("============================ myInt = 65");
		myInt = 65;
		$display(">|         65|");
		$display("*|",myInt,"|");

		$display(">|00000041|");
		$displayh("*|",myInt,"|");

		$display(">|00000000101|");
		$displayo("*|",myInt,"|");

		$display(">|00000000000000000000000001000001|");
		$displayb("*|",myInt,"|");
	end
endmodule
