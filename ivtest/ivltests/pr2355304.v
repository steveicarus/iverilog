module test ();
    parameter t=0;
    reg t_not, t_zero;

    generate
    if (!t) begin
        initial t_not = 1;
    end
    endgenerate

    generate
    if (t==0) begin
        initial t_zero = 1;
    end
    endgenerate

    initial begin
	#1 if (t_not !== 1) begin
		$display("FAILED -- t_not=%b", t_not);
		$finish;
	end
	if (t_zero !== 1) begin
		$display("FAILED -- t_zero=%b", t_zero);
		$finish;
	end
	$display("PASSED");
    end
endmodule
