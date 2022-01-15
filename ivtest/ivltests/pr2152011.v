module dupe;

integer cc;
reg signed [19:0] y;
initial for (cc=0; cc<20; cc=cc+1) begin
	y = $floor(200000.0*$sin(cc*0.81)+0.5);
	$display("%7d", y);
end

endmodule
