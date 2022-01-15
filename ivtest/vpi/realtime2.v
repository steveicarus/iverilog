`timescale 1 ns / 1 ps
module test;
    initial begin
	#12.3456;
	$display("$time = %0t", $time);
	$test(test);
	#34.5678;
	$display("$time = %0t", $time);
	$test(test);
    end
endmodule

`timescale 1 ps / 1 ps
module test2;
    initial begin
	#56.7890;
	$display("$time = %0t", $time);
	$test(test2);
	#78.9012;
	$display("$time = %0t", $time);
	$test(test2);
    end
endmodule
