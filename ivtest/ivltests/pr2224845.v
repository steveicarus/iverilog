`define VUG_PCREL(u, uch) ({ {(uch - 12 - 1 > 0 ? uch - 12 - 1 : 1){u[11]}}, \
                             u[10:0], 2'b00 })

module t();

parameter uch = 16;
parameter u_hossz = 32;
parameter u_prefix = 3;

reg [u_hossz - u_prefix - 1:0] v_utas;
reg [uch - 1:0] v_cim;
wire [uch - 1:0] v_ugras_ide;

assign v_ugras_ide = v_cim + `VUG_PCREL(v_utas, uch);

initial
begin
	v_utas = 'h0fff;
	v_cim = 'h7;
	#1;

	if(v_ugras_ide !== 'h3)
		$display("FAILED");
	else
		$display("PASSED");

	$finish;
end

endmodule
