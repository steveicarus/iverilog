module dpram #(
	parameter aw=8,
	parameter dw=8
) (input clka, clkb, wena,
	input [aw-1:0] addra, addrb,
	input [dw-1:0] dina,
	output [dw-1:0] doutb
);
// minimalist dual-port RAM model, hope most tools can synthesize it
localparam sz=(32'b1<<aw)-1;
reg [dw-1:0] mem[sz:0];
reg [aw-1:0] alb=0;
(* ivl_synthesis_on *)
always @(posedge clka) if (wena) mem[addra] <= dina;
always @(posedge clkb) alb <= addrb;
assign doutb = mem[alb];
(* ivl_synthesis_off *)
initial $display("PASSED");
endmodule
