/*
 * Verification test for increment/decrement operators
 *
 * Author:  Prasad Joshi <prasad@canopusconsultancy.com>
 */

module main;
logic la;
logic lb;

int ia;
int ib;

bit ba;
bit bb;

real ra;
real rb;
real rc;

	initial begin

		/* logic tests */
		la = 0;
		#1

		lb = ++la;
		#1
		if (la != lb) begin
			$display("FAILED");
			$finish;
		end

		ib = 15;
		#1
		ia = ++ib;
		#1
		if (ia != ib) begin
			$display("FAILED");
			$finish;
		end

		ia = 15;
		#1
		ib = ia++;
		#1
		if (ia != 16 || ib != 15) begin
			$display("FAILED");
			$finish;
		end

		ib = --ia;
		if (ib != ia) begin
			$display("FAILED");
			$finish;
		end

		/* bit test */
		ba = 0;
		#1
		for (ia = 0; ia < 10; ia = ia + 1) begin
			bb = --ba;
			#1
			if (bb != ba && !(bb == 1 || bb == 0)) begin
				$display("FAILED");
				$finish;
			end
		end

		/* real decrement test */
		ia = 15;
		ra = --ia;
		if (ra != ia) begin
			$display("FAILED");
			$finish;
		end

		rb = 19.99;
		rc = rb - 2;
		ra = --rb;
		if (ra != rb) begin
			$display("FAILED");
			$finish;
		end

		ra = rb--;
		if (ra == rb || rc != rb) begin
			$display("FAILED");
			$finish;
		end

		/* real increment test */
		ia = 15;
		ra = ++ia;
		if (ra != ia) begin
			$display("FAILED");
			$finish;
		end

		rb = 19.99;
		rc = rb + 2;
		ra = ++rb;
		if (ra != rb) begin
			$display("FAILED");
			$finish;
		end

		ra = rb++;
		if (ra == rb || rc != rb) begin
			$display("FAILED");
			$finish;
		end

		$display("PASSED");
	end
endmodule
