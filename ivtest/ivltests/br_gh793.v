// Check that $signed/$unsigned works when being combinatorially assigned with a
// delay and the target of the function is a net without any drivers.

module top ();
	wire [7:0] a;
	wire signed [7:0] b;
	assign #1 b = $signed(a);

	initial begin
		#10
		if (b === 8'hzz) begin
			$display("PASSED");
		end else begin
			$display("FAILED");
		end
	end
endmodule
