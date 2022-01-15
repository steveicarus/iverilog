`begin_keywords "1364-2005"
`timescale 1ns/1ns
module test;

	reg fail = 0;

	task check;
		input [10*8:1] expect;
		reg [10*8:1] s;
		begin
			$swrite(s, "Time %t", $time);
			$write("%s", s);
			if (s === expect)
				$display("");
			else
			begin
				$display(" != %s", expect);
				fail = 1;
			end
		end
	endtask


	initial
	begin
		$display("Test display formatting of time values");
		$timeformat(-6, 3, " us", 20);

		fork
			#0000 check("  0.000 us");
			#0001 check("  0.001 us");
			#0010 check("  0.010 us");
			#0011 check("  0.011 us");
			#0100 check("  0.100 us");
			#0101 check("  0.101 us");
			#1000 check("  1.000 us");
			#1001 check("  1.001 us");
		join

		$display("%s", fail? "FAILED" : "PASSED");
	end

endmodule
`end_keywords
