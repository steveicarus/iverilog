/*
 * From PR#751.
 * The (*) can get tangled with a contracted (* *)
 */
module tb;
reg [1:0] sel;
reg [0:3] in;
reg out;
always @(*)
	out = in[sel];
initial
begin
	$monitor($time, " %b[%b]: %b", in, sel, out);
	#10 in = 4'b 0100;
	#10 sel = 0;
	#10 sel = 1;
	#10 $finish(0);
end
endmodule
