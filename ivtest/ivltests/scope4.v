module test4;
    reg r;
endmodule

module test3;
    test4 test4();
endmodule

module test2;
    initial begin
	$dumpvars(1, test3.test4);
    end
    test3 test3();
endmodule

module test;
    initial begin
	$dumpfile("work/dumpfile.vcd");
	$dumpvars(1, test2.test3);
	$display("PASSED");
    end
    test2 test2();
endmodule
