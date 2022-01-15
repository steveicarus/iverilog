module MULT32;
    wire VDD = 1 ;
    wire VSS = 0 ;
    wire X00, X31 ;
    reg [1:0] state ;
    assign {X31, X00} = state ;

    ADDERXY XX1 (VSS, N1110, VSS, N2166, X00, X31);	/* This one! */

    ADDERXY XX127 (N1076, N1044, N2131, N2100, VSS, VSS);

    initial
	begin
	state = 0 ;
	$monitor( X00, X31, VSS, VSS,,, N2166, N1110 ) ;
	#10 state = 1 ;
	#10 state = 0 ;
	#10 state = 1 ;
	#10 state = 0 ;
	#10 $finish(0);
	end

endmodule


module ADDERXY( cin, cout, b, sum, x, y );
    input cin, b, x, y;
    output cout, sum;
    reg [1:0] total ;

    assign sum = total[0] ;
    assign cout = total[1] ;

    always @ (x or y or b or cin)
	begin
	    total = ( x & y ) + b + cin ;
	end
endmodule // ADDERXY
