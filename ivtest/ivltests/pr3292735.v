module table_out(
	input [1:0] a,
	(* rom_style = "distributed" *) output reg signed [9:0] phase
);

always @(*) case (a)
	2'd 0: phase =    0;
	2'd 1: phase =   90;
	2'd 2: phase =  180;
	2'd 3: phase =  270;
endcase

initial $display("PASSED");

endmodule
