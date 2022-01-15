module main;
	real x;

	initial
		begin
			x = 1.0;

			$display("Hello, World");
			$display("Positive x is %f", x);
			$display("-1.0 * x is %f", -1.0 * x);
			$display("0.0 - x is %f", 0.0 - x);
			$display("-x is %f", -x);

			$finish(0);
		end
endmodule
