module et1;
reg [31:0] a;
reg [31:0] b;

wire [31:0] x;
reg [31:0] y;

event e1;

initial begin
//	$dumpvars;
	$monitor ("T=", $time, ", a=", a, ", b=", b, ", x=",
x, ", y=", y);
	#200
	$finish(0);
end

initial begin
	a = 10;
	b = 20;
	#10
	a = 30;
	#10
	b = 40;
	#10
	a = 50;
	-> et1.m1.e2;
	#10
	b = 60;
	-> et1.m1.e2;
	#10
	a = 70;
	-> et1.m1.e2;
	b = 80;
	#10
	a = 90;
end

always @e1 begin
	y <= b;
end


m m1 (a,x);

endmodule

module m (a,x);
input [31:0] a;
output [31:0] x;
reg    [31:0] x;

event e2;

always @e2 begin
	#1
	x <= a;
	#2
	-> et1.e1;
end
endmodule
