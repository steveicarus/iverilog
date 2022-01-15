`begin_keywords "1364-2005"
/*
 * This program is based on PR#1077.
 *
 * Expected output:
 *  one
 *  y = x
 *  one
 *  y = 1
 *  one
 *  y = 1
 */
module bool;

reg clk,y;
reg [31:0] count;

initial clk=0;
always #2.5 clk = ~clk;

initial begin
	count = 'h8;
	#20
	$finish(0);
end

always @(posedge clk)
begin
	// BUG: this should eval to "1" but does not!
	y <= count[10] || ~count[5:3];

	// this should print "one" and does
	if(count[10] || ~(count[5:3]))
		$display("one");
	else
		$display("zero");
	$display("y = %b",y);

end

endmodule
`end_keywords
