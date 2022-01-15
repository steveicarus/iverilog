// $ iverilog -Wall simpler.v -o simpler
// $ vvp simpler
// simpler:37: syntax error

`timescale 1ns / 1ns

module simpler;

reg [1:0] cnt=0;
wire result;

defparam \Mcount_cnt_xor<3>11 .INIT = 4'hC;
test_lut \Mcount_cnt_xor<3>11  (
	.a0(cnt[0]),
	.a1(cnt[1]),
	.O(result)
);

initial $display("PASSED");

endmodule


module test_lut (output O, input a0, input a1);
	parameter INIT = 4'h0;

	reg tmp;
	always @(*) tmp = mux ( INIT, {a1, a0});
	assign O = tmp;

	function mux;
		input [3:0] d;
		input [1:0] s;
		mux = d[s];
	endfunction

endmodule
