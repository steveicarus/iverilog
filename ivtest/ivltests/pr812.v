////////////////////////////////////////////////////////////////////////
// Copyright 2003 University of Kentucky
//
// This file is released into the public domain
////////////////////////////////////////////////////////////////////////

//
// Top level module
//
module top();
	parameter tp = 'd1;

	reg a;
	wire b;

	bot b1(b, a);

	initial begin
		a = 0;
		$display("tp = %d in top", tp);
	end
endmodule


//
// bottom level module
//

`define div 0.100

module bot(a, b);
	input b;
	output a;

	real tp;	// tp is overridden by tp parameter in top
	real tp2;

	assign a = b;
	initial begin
		tp = 1 / `div;
		tp2 = 1 / `div;
		$display("tp = %f, tp2 = %f", tp, tp2);
		if (tp != 10.0)
			$display("tp != 10.0.  (tp = %f)", tp);
		else
			$display("tp == 10, (expected)");
		#1 $display("tp = %f, tp2 = %f", tp, tp2);
	end
endmodule
