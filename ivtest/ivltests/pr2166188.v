module t();

parameter eh = 11;
parameter mh = 52;

parameter ih2 = 6;
parameter fh = 7;

localparam ih = 1 << ih2;

reg [ih - 1:0] i_abs;

reg at;
reg [ih2 - 1:0] fls;

wire [ih - 1:0] i_norm;

assign i_norm = i_abs << (at ? ih - mh - 1 : fls);

initial
begin
	at = 1;
	fls = 123;
	i_abs = 'h123;
	#1;
	if(i_norm !== ('h123 << 11))
		$display("FAILED");
	else
		$display("PASSED");
end

endmodule
