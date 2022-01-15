module test;

    parameter parm1 = 0;
    parameter parm2 = parm1 == 0;

    initial begin
	// if got here then we compiled
	if (parm2)
	    $display("PASSED");
	else
	    $display("FAILED");
    end

endmodule
