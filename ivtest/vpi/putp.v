module test;
    reg [15:0] r;
    integer i, i2;
    real r1, r2;
    initial begin
	i = $mytest(r, i2, r1, r2);

	if (i !== 69 || r !== 11 || i2 !== 22 || r1 != 3.3 || r2 != 4.4) begin
	    $display("i = %0d, r = %0d, i2 = %0d, r1 = %f, r2 = %f",
		i, r, i2, r1, r2);
	    $display("FAILED");
	end else
	    $display("PASSED");
    end
endmodule
