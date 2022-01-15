module tern;

reg [13:0] fdbk_err_wide;

// arithmetic saturation from 14 bits down to 13 bits, and drop lsb
wire [11:0] fdbk_err = ((fdbk_err_wide[13:12]==2'b00) | (fdbk_err_wide[13:12]==2'b11)) ?
       fdbk_err_wide[12:1] : {fdbk_err_wide[13],{11{~fdbk_err_wide[13]}}};

initial begin
	#10;
	$display(fdbk_err_wide, fdbk_err);
	// 2008-03-04 snapshot prints x x, 2008-04-02 git prints x z
	// 01eb298228d0adce9d62818e21d47fb274af9060 is first "bad" commit
	#10;
	fdbk_err_wide = 42;
	#10;
	$display(fdbk_err_wide, fdbk_err);
	// everybody agrees this is 42 21
	#10;
	fdbk_err_wide = 14'bxxxxxxxxxxxxxx;
	#10;
	// everybody agrees this is x x
	$display(fdbk_err_wide, fdbk_err);
	#10;
	$finish(0);
end

endmodule
