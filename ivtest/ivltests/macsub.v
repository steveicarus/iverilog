module test();

integer j;
integer jel;
integer x;
integer xel;

`define A(j) (jel == 1 && j == 2)
`define B(j) (jel \
 == 1 && \
 j == 2)

initial
begin
	j = 0;
	jel = 1;
	x = 2;
	xel = 3;

	if(`A(x) && `B(x))
		$display("PASSED");
	else
		$display("FAILED");
end
endmodule
